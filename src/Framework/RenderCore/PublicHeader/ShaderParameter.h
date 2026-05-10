#pragma once

#include "RenderGraphResource.h"
#include "Math.hpp" // For Float2, Float3, Float4, etc.
#include "RHIDefine.h"
#include "RHICommandList.h"
#include "ShaderCore.h"
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
        static constexpr EShaderParameterBaseType BaseType = EShaderParameterBaseType::Unknown;
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
        static constexpr EShaderParameterBaseType BaseType = EShaderParameterBaseType::RDGTexture;
        static constexpr uint32_t NumRows = 1;
        static constexpr uint32_t NumColumns = 1;
        static constexpr uint32_t NumElements = 0;
        static constexpr uint32_t Alignment = 0; // 资源通常不占 ConstantBuffer 空间
        static constexpr bool bIsStoredInConstantBuffer = false; // 关键：标记为非 CBuffer 成员

        using TAlignedType = RenderGraphTexture*;

        static const ShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct ShaderParameterTypeInfo<RenderGraphTextureUAV>
    {
        static constexpr EShaderParameterBaseType BaseType = EShaderParameterBaseType::RDGTexture_UAV;
        static constexpr uint32_t NumRows = 1;
        static constexpr uint32_t NumColumns = 1;
        static constexpr uint32_t NumElements = 0;
        static constexpr uint32_t Alignment = 0; // 资源通常不占 ConstantBuffer 空间
        static constexpr bool bIsStoredInConstantBuffer = false; // 关键：标记为非 CBuffer 成员

        using TAlignedType = RenderGraphTextureUAV*;

        static const ShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct ShaderParameterTypeInfo<RHI::RHISampler>
    {
        static constexpr EShaderParameterBaseType BaseType = EShaderParameterBaseType::RHISampler;
        static constexpr uint32_t NumRows = 1;
        static constexpr uint32_t NumColumns = 1;
        static constexpr uint32_t NumElements = 0;
        static constexpr uint32_t Alignment = 0; // 资源通常不占 ConstantBuffer 空间
        static constexpr bool bIsStoredInConstantBuffer = false; // 关键：标记为非 CBuffer 成员

        using TAlignedType = RHI::RHISampler*;

        static const ShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct ShaderParameterTypeInfo<float>
    {
        static constexpr EShaderParameterBaseType BaseType = EShaderParameterBaseType::Float32;
        static constexpr uint32_t NumRows = 1;
        static constexpr uint32_t NumColumns = 1;
        static constexpr uint32_t NumElements = 0;
        static constexpr uint32_t Alignment = 0; // 资源通常不占 ConstantBuffer 空间
        static constexpr bool bIsStoredInConstantBuffer = false; // 关键：标记为非 CBuffer 成员

        using TAlignedType = float;

        static const ShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };
    template<>
    struct ShaderParameterTypeInfo<Core::Float2>
    {
        static constexpr EShaderParameterBaseType BaseType = EShaderParameterBaseType::Float32;
        static constexpr uint32_t NumRows = 1;
        static constexpr uint32_t NumColumns = 2;
        static constexpr uint32_t NumElements = 0;
        static constexpr uint32_t Alignment = 0; // 资源通常不占 ConstantBuffer 空间
        static constexpr bool bIsStoredInConstantBuffer = false; // 关键：标记为非 CBuffer 成员

        using TAlignedType = Core::Float2;

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
            EShaderParameterBaseType BaseType;
            uint32_t NumRows = 1;
            uint32_t NumColumns = 1;
            uint32_t NumElements = 0;
            const ShaderParametersMetadata* StructMetadata = nullptr;
            Member(
            const char* InName,
            uint32_t InOffset,
            EShaderParameterBaseType InBaseType,
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
            bool IsResource() const { return BaseType >= EShaderParameterBaseType::RDGTexture; }
        };
		bool InitFlag = false;
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


        std::vector<Member> Members;
    private:
        const char* StructName;
        uint32_t Size;
        
        EUseCase UseCase;

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
        if(!sMetaData.InitFlag){\
        std::vector<ShaderParametersMetadata::Member>& Members = sMetaData.Members;\
        FuncPtr(*PrevFunc)(LastIdType, std::vector<ShaderParametersMetadata::Member>*);            \
        PrevFunc = sAppendMemberGetPrev; \
        FuncPtr func = (FuncPtr)PrevFunc; \
        do{\
            func = reinterpret_cast<MemberFuncType>(func)(LastIdType(), &Members);\
	    } while (func != nullptr); \
        sMetaData.InitFlag = true;\
        }\
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
	typedef CurMemberIdType##MemberName

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

#define SHADER_PARAMETER_SAMPLER(MemberName) \
    SHADER_PARAMETER_INTERNAL( \
        ShaderParameterTypeInfo<RHI::RHISampler>::BaseType, \
        ShaderParameterTypeInfo<RHI::RHISampler>::TAlignedType, \
        MemberName, \
        ShaderParameterTypeInfo<RHI::RHISampler>)

#define SHADER_PARAMETER_TEXTURE_UAV(TextureUAVType, MemberName) \
    SHADER_PARAMETER_INTERNAL( \
        ShaderParameterTypeInfo<TextureUAVType>::BaseType, \
        ShaderParameterTypeInfo<TextureUAVType>::TAlignedType, \
        MemberName, \
        ShaderParameterTypeInfo<TextureUAVType>)

BEGIN_SHADER_PARAMETER_STRUCT(A)
    SHADER_PARAMETER(Core::Int2,Color)
END_SHADER_PARAMETER_STRUCT(A)

class Shader;
RENDERCORE_API void SetShaderParameters(
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