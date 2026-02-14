#pragma once

#include "RenderGraphResource.h"
#include "Math.hpp" // For Float2, Float3, Float4, etc.
#include "RHIDefine.h"
#include "Shader.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <iostream>

#define SHADER_PARAMETER_ALIGNMENT 16

namespace RenderCore {


    class ShaderParametersMetadata;

    template<typename T>
    struct ShaderParameterTypeInfo
    {
        static constexpr EShaderUniformBaseType BaseType = EShaderUniformBaseType::Unknown;
        static constexpr uint32_t NumRows = 1;
        static constexpr uint32_t NumColumns = 1;
        static constexpr uint32_t NumElements = 0;

        static constexpr uint32_t Alignment = SHADER_PARAMETER_ALIGNMENT;
        static constexpr bool bIsStoredInConstantBuffer = true;

        using TAlignedType = T;

        static const ShaderParametersMetadata* GetStructMetadata()
        {
            return nullptr;
        }
    };

    class ShaderParametersMetadata
    {
    public:
        enum class EUseCase : uint8_t
        {
            ShaderParameterStruct,
            UniformBuffer
        };

        struct Member
        {
            const char* Name;
            uint32_t Offset;
            EShaderUniformBaseType BaseType;
            uint32_t NumRows = 1;
            uint32_t NumColumns = 1;
            uint32_t NumElements = 0;
            const ShaderParametersMetadata* StructMetadata = nullptr;
            Member(
            const char* InName,
            uint32_t InOffset,
            EShaderUniformBaseType InBaseType,
            uint32_t InNumRows,
            uint32_t InNumColumns,
            uint32_t InNumElements,
            const ShaderParametersMetadata* InStructMetadata
            ):
				Name(InName), 
                Offset(InOffset), 
                BaseType(InBaseType), 
                NumRows(InNumRows), 
                NumColumns(InNumColumns), 
                NumElements(InNumElements), 
                StructMetadata(InStructMetadata)
            
            {

            }

            bool IsStruct() const { return StructMetadata != nullptr; }
            bool IsResource() const { return BaseType >= EShaderUniformBaseType::Texture; }
        };

    public:

        ShaderParametersMetadata(
            const char* InStructName,
            uint32_t InSize,
            std::vector<Member> InMembers,
            EUseCase InUseCase = EUseCase::ShaderParameterStruct)
            : StructName(InStructName)
            , Size(InSize)
            , Members(std::move(InMembers))
            , UseCase(InUseCase)
        {
        }

        const char* GetStructName() const { return StructName; }
        uint32_t GetSize() const { return Size; }
        const std::vector<Member>& GetMembers() const { return Members; }
        EUseCase GetUseCase() const { return UseCase; }

        // ---------------------------
        // HLSL generation
        // ---------------------------
        void GenerateHLSL(std::ostream& Out, uint32_t& cbufferRegister, int indent = 0) const
        {
            std::string IndentStr(indent, ' ');

            if (UseCase == EUseCase::UniformBuffer)
            {
                Out << IndentStr << "cbuffer " << StructName << " : register(b" << cbufferRegister++ << ")\n";
                Out << IndentStr << "{\n";
            }

            for (const auto& Member : Members)
            {
                if (Member.IsStruct())
                {
                    // 嵌套结构单独生成 cbuffer
                    Member.StructMetadata->GenerateHLSL(Out, cbufferRegister, indent + 4);
                    continue;
                }

                Out << IndentStr << "    " << GetHLSLType(Member) << " " << Member.Name;

                if (Member.NumElements > 0)
                    Out << "[" << Member.NumElements << "]";

                Out << ";\n";
            }

            if (UseCase == EUseCase::UniformBuffer)
            {
                Out << IndentStr << "};\n\n";
            }
        }

    private:
        const char* StructName;
        uint32_t Size;
        std::vector<Member> Members;
        EUseCase UseCase;

        static std::string GetHLSLType(const Member& M)
        {
            switch (M.BaseType)
            {
            case EShaderUniformBaseType::Float32:
                if (M.NumRows == 1 && M.NumColumns == 1) return "float";
                if (M.NumRows == 1 && M.NumColumns == 2) return "float2";
                if (M.NumRows == 1 && M.NumColumns == 3) return "float3";
                if (M.NumRows == 1 && M.NumColumns == 4) return "float4";
                if (M.NumRows == 4 && M.NumColumns == 4) return "float4x4";
                return "float";
            case EShaderUniformBaseType::Int32: return "int";
            case EShaderUniformBaseType::UInt32: return "uint";
            case EShaderUniformBaseType::Texture: return "Texture2D";
            case EShaderUniformBaseType::Texture_UAV: return "RWTexture2D<float4>";
            case EShaderUniformBaseType::Buffer: return "StructuredBuffer<float4>";
            case EShaderUniformBaseType::Buffer_UAV: return "RWStructuredBuffer<float4>";
            case EShaderUniformBaseType::Sampler: return "SamplerState";
            default: return "unknown";
            }
        }
    };
    

#define STRUCT_OFFSET(StructType, Member) offsetof(StructType, Member)
// Begin shader parameter struct
#define BEGIN_SHADER_PARAMETER_STRUCT(StructClass) struct StructClass{ \
    struct FirstIdType{}; \
    using PrevMemberIdType = FirstIdType; \
    using ThisStructType = StructClass; \
    using FuncPtr = void*;\
    typedef FuncPtr(*MemberFuncType)(PrevMemberIdType, std::vector<ShaderParametersMetadata::Member>*);\
    static FuncPtr sAppendMemberGetPrev(PrevMemberIdType, std::vector<ShaderParametersMetadata::Member>*) \
		{ \
			return nullptr; \
		} \
	typedef PrevMemberIdType
// End shader parameter struct
#define END_SHADER_PARAMETER_STRUCT(StructClass) \
    LastIdType;\
    static std::vector<ShaderParametersMetadata::Member> GetMetaData() \
    {\
        std::vector<ShaderParametersMetadata::Member> Members;\
        FuncPtr(*PrevFunc)(LastIdType, std::vector<ShaderParametersMetadata::Member>*);            \
        PrevFunc = sAppendMemberGetPrev; \
        FuncPtr func = (FuncPtr)PrevFunc; \
        do{\
            func = reinterpret_cast<MemberFuncType>(func)(LastIdType(), &Members);\
	    } while (func != nullptr); \
        return Members;\
    }\
};

#define SHADER_PARAMETER_INTERNAL(BaseType,MemberType,MemberName,TypeInfo)\
    PrevType##Name;\
    struct CurMember##Name : PrevType##Name{};\
    using CurMemberIdType = CurMember##Name;\
public:\
    MemberType MemberName;\
private:\
    static FuncPtr sAppendMemberGetPrev(CurMemberIdType, std::vector<ShaderParametersMetadata::Member>* Members) \
		{ \
            Members->push_back(ShaderParametersMetadata::Member(\
            #MemberName,\
            STRUCT_OFFSET(ThisStructType, MemberName),\
            BaseType,\
            TypeInfo::NumRows,\
            TypeInfo::NumColumns,\
            TypeInfo::NumElements,\
            TypeInfo::GetStructMetadata()));\
			FuncPtr(*PrevFunc)(PrevType##Name, std::vector<ShaderParametersMetadata::Member>*); \
			PrevFunc = sAppendMemberGetPrev; \
            return (FuncPtr)PrevFunc; \
		} \
	typedef PrevMemberIdType

// Define a texture parameter
#define SHADER_PARAMETER(ClassType,Name) \
SHADER_PARAMETER_INTERNAL(ShaderParameterTypeInfo<ClassType>::BaseType,ClassType,Name,ShaderParameterTypeInfo<ClassType>)

BEGIN_SHADER_PARAMETER_STRUCT(A)
    SHADER_PARAMETER(Core::Int2,Color)
END_SHADER_PARAMETER_STRUCT(A)


struct RENDERCORE_API ShaderParameterInstance
{
    EShaderUniformBaseType Type;
    void* Ptr = nullptr;   // 指向实际参数内存
    size_t Size = 0;
};


//class RENDERCORE_API ShaderParameterReader
//{
//public:
//    ShaderParameterReader(void* rawParameters, const ShaderMetaData& metaData)
//        : RawParameters(reinterpret_cast<uint8_t*>(rawParameters))
//        , ParameterMetaData(metaData)
//    {
//        Reset();
//    }
//
//    void Reset()
//    {
//        Stack.clear();
//        IndexStack.clear();
//        Stack.push_back(&ParameterMetaData);
//        IndexStack.push_back(0);
//    }
//
//    // 返回下一个非 struct 参数
//    bool ReadNext(ShaderParameterInstance& outParam)
//    {
//        while (!Stack.empty())
//        {
//            const ShaderMetaData* currentMeta = Stack.back();
//            size_t& currentIndex = IndexStack.back();
//
//            // 遍历 struct 的 Nested
//            if (currentIndex < currentMeta->Nested.size())
//            {
//                const ShaderMetaData& memberMeta = currentMeta->Nested[currentIndex++];
//
//                if (memberMeta.Type == EShaderUniformBaseType::Struct)
//                {
//                    // 进入 struct，递归展开
//                    Stack.push_back(&memberMeta);
//                    IndexStack.push_back(0);
//                    continue;
//                }
//
//                // 非 struct，返回参数
//                outParam.Type = memberMeta.Type;
//                outParam.Ptr = RawParameters + memberMeta.Offset;
//                outParam.Size = memberMeta.Size;
//                return true;
//            }
//            else
//            {
//                // struct 遍历完成，弹出
//                Stack.pop_back();
//                IndexStack.pop_back();
//            }
//        }
//
//        // 没有更多参数
//        return false;
//    }
//
//    std::map<std::string,ShaderParameter*> GetAllParameterMaps() {
//        std::map<std::string, ShaderParameter*> result;
//        ShaderParameterInstance param;
//        while (ReadNext(param)) {
//            auto parameter = static_cast<ShaderParameter*>(param.Ptr);
//            auto name = parameter->GetName();
//            result[name] = parameter;
//        }
//        return result;
//    }
//
//private:
//    uint8_t* RawParameters = nullptr;
//    const ShaderMetaData& ParameterMetaData;
//
//    // 用栈保存当前递归 struct 的状态
//    std::vector<const ShaderMetaData*> Stack;
//    std::vector<size_t> IndexStack;
//};
//
//
class RENDERCORE_API ShaderParameterStruct {
public:
    ShaderParameterStruct() = default;
    template<typename T>
    ShaderParameterStruct(const T* content) : Paramters((void*)content) {
        ContentMetaData = T::GetMetaData();
    }
    template<typename T>
    void operator=(const T* content) {
        Paramters = (void*)content;
        ContentMetaData = T::GetMetaData();
    }

    void* Paramters = nullptr;
    ShaderParametersMetadata ContentMetaData;
    // Set shader parameters into command list for the given shader
    void SetShaderParameters(RHI::RHICommandList& cmdList, const ShaderSP& shader,ShaderParameterBindingInfo* bindingInfo) const
    {
        if (!Paramters || !shader)
            return;

        if (!bindingInfo)
            return;

        // Create batched shader parameters
        RHI::RHIBatchedShaderParameters batchedParams;

        //auto allShaderParameters = ShaderParameterReader(Paramters, ///ContentMetaData).GetAllParameterMaps();
        //
        //// Traverse ContentMetaData and match with binding info
        //for (const auto& paramInfo : allShaderParameters)
        //{
        //    //const ShaderParameterBinding* binding = bindingInfo->GetParameterBind(paramInfo.first);
        //    //if (!binding)
        //    //    continue; // Skip unmatched parameters
        //    //auto parameter = paramInfo.second;
        //
        //}

        // Set the batched parameters into the command list
        cmdList.SetBatchedShaderParameters(shader->GetRHIShader(), batchedParams);
    }
};


} // namespace WR::RenderCore