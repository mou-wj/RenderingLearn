#pragma once

#include "RenderGraphResource.h"
#include "Math.hpp" // For Float2, Float3, Float4, etc.
#include "RHIDefine.h"
#include "RHICommandList.h"
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

    template<>
    struct ShaderParameterTypeInfo<RenderGraphTexture>
    {
        static constexpr EShaderUniformBaseType BaseType = EShaderUniformBaseType::Texture;
        static constexpr uint32_t NumRows = 1;
        static constexpr uint32_t NumColumns = 1;
        static constexpr uint32_t NumElements = 0;
        static constexpr uint32_t Alignment = 0; // 资源通常不占 ConstantBuffer 空间
        static constexpr bool bIsStoredInConstantBuffer = false; // 关键：标记为非 CBuffer 成员

        using TAlignedType = RenderGraphTexture*;

        static const ShaderParametersMetadata* GetStructMetadata() { return nullptr; }
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
        void GenerateHLSL(std::ostream& Out, uint32_t& bindingSlot, int indent = 0) const
        {
            std::string IndentStr(indent, ' ');

            // 1. 如果是 UniformBuffer (HLSL 中对应 ConstantBuffer)
            if (UseCase == EUseCase::UniformBuffer)
            {
                // 使用 Vulkan 风格的 HLSL 绑定语法
                // [[vk::binding(X)]] 是最显式的写法，或者使用标准的 : register(bX)
                Out << IndentStr << "cbuffer " << StructName << " : register(b" << bindingSlot++ << ")\n";
                Out << IndentStr << "{\n";
            }
            else if (indent == 0) // 顶级普通结构体定义
            {
                Out << IndentStr << "struct " << StructName << "\n";
                Out << IndentStr << "{\n";
            }

            // 2. 处理常量数据成员 (变量)
            for (const auto& Member : Members)
            {
                if (Member.IsResource()) continue; // 资源（纹理/采样器）不进 cbuffer

                if (Member.IsStruct())
                {
                    // 嵌套结构体引用
                    Out << IndentStr << "    " << Member.StructMetadata->GetStructName() << " " << Member.Name << ";\n";
                }
                else
                {
                    Out << IndentStr << "    " << GetHLSLType(Member) << " " << Member.Name;
                    if (Member.NumElements > 0)
                        Out << "[" << Member.NumElements << "]";
                    Out << ";\n";
                }
            }

            // 3. 闭合结构/cbuffer
            if (UseCase == EUseCase::UniformBuffer)
            {
                // 实例名：为了让 VS/PS 逻辑一致，建议给 cbuffer 一个实例名
                // 这样在 Shader 中访问就是 Primitive.LocalToWorld
                std::string InstanceName = StructName;
                if (InstanceName[0] == 'F' && isupper(InstanceName[1])) InstanceName.erase(0, 1);

                Out << IndentStr << "} " << InstanceName << ";\n\n";
            }
            else if (indent == 0)
            {
                Out << IndentStr << "};\n\n";
            }

            // 4. 处理资源成员 (Texture / Sampler / Buffer)
            // 这些在 HLSL 中必须定义在 ConstantBuffer 外部
            for (const auto& Member : Members)
            {
                if (Member.IsResource())
                {
                    // 根据类型选择寄存器前缀 (t 为纹理/Buffer, s 为采样器, u 为 UAV)
                    char regChar = 't';
                    if (Member.BaseType == EShaderUniformBaseType::Sampler) regChar = 's';
                    else if (Member.BaseType == EShaderUniformBaseType::Texture_UAV ||
                        Member.BaseType == EShaderUniformBaseType::Buffer_UAV) regChar = 'u';

                    Out << "layout(binding = " << bindingSlot << ") " // 兼容 SPIR-V 编译
                        << GetHLSLType(Member) << " " << Member.Name
                        << " : register(" << regChar << bindingSlot << ");\n";

                    bindingSlot++;
                }
            }
        }
    
        std::vector<Member> Members;
    private:
        const char* StructName;
        uint32_t Size;
        
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
public:\
    static const ShaderParametersMetadata& GetMetaData() \
    {\
        static ShaderParametersMetadata sMetaData(#StructClass,sizeof(StructClass),{});\
        std::vector<ShaderParametersMetadata::Member>& Members = sMetaData.Members;\
        FuncPtr(*PrevFunc)(LastIdType, std::vector<ShaderParametersMetadata::Member>*);            \
        PrevFunc = sAppendMemberGetPrev; \
        FuncPtr func = (FuncPtr)PrevFunc; \
        do{\
            func = reinterpret_cast<MemberFuncType>(func)(LastIdType(), &Members);\
	    } while (func != nullptr); \
        return sMetaData;\
    }\
};

#define SHADER_PARAMETER_INTERNAL(BaseType,MemberType,MemberName,TypeInfo)\
    PrevType##MemberName;\
    struct CurMember##MemberName : PrevType##MemberName{};\
    using CurMemberIdType##MemberName = CurMember##MemberName;\
public:\
    MemberType MemberName;\
private:\
    static FuncPtr sAppendMemberGetPrev(CurMemberIdType##MemberName, std::vector<ShaderParametersMetadata::Member>* Members) \
		{ \
            Members->push_back(ShaderParametersMetadata::Member(\
            #MemberName,\
            STRUCT_OFFSET(ThisStructType, MemberName),\
            BaseType,\
            TypeInfo::NumRows,\
            TypeInfo::NumColumns,\
            TypeInfo::NumElements,\
            TypeInfo::GetStructMetadata()));\
			FuncPtr(*PrevFunc)(PrevType##MemberName, std::vector<ShaderParametersMetadata::Member>*); \
			PrevFunc = sAppendMemberGetPrev; \
            return (FuncPtr)PrevFunc; \
		} \
	typedef PrevMemberIdType

// Define a texture parameter
#define SHADER_PARAMETER(ClassType,Name) \
SHADER_PARAMETER_INTERNAL(ShaderParameterTypeInfo<ClassType>::BaseType,ClassType,Name,ShaderParameterTypeInfo<ClassType>)

    // 定义纹理参数宏
#define SHADER_PARAMETER_TEXTURE(TextureType, MemberName) \
    SHADER_PARAMETER_INTERNAL( \
        ShaderParameterTypeInfo<TextureType>::BaseType, \
        ShaderParameterTypeInfo<TextureType>::TAlignedType, \
        MemberName, \
        ShaderParameterTypeInfo<TextureType>)

BEGIN_SHADER_PARAMETER_STRUCT(A)
    SHADER_PARAMETER(Core::Int2,Color)
END_SHADER_PARAMETER_STRUCT(A)

class Shader;
void SetShaderParameters(
    RHI::RHICommandListBase& cmdList,
    const Shader* shader,
    const ShaderParametersMetadata& ParametersMetaData,
    void* ParametersData);

/**
 * RenderCore 层函数：将 C++ 结构体参数提交至 RHI 指令流
 * @param cmdList  RHI 指令列表
 * @param shader   Shader 对象（持有 Metadata 和 Binding 映射）
 * @param ParametersData C++ 结构体实例指针
 */
template<typename TParameters>
void SetShaderParameters(
    RHI::RHICommandListBase& cmdList,
    const Shader* shader,
    const TParameters& ParametersData) {
    SetShaderParameters(cmdList, shader, ParametersData.GetMetaData(), (void*)&ParametersData);
}




} // namespace WR::RenderCore