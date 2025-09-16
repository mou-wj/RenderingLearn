#pragma once

#include "RenderGraphResource.h"
#include "Math.hpp" // For Float2, Float3, Float4, etc.


#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <type_traits>

namespace RenderCore {
    using namespace Common;


    // 参数类型枚举
    enum class EShaderParamType {
        Float,
        Float2,
        Float3,
        Float4,
        Matrix4x4,
        Texture2D,
        Sampler,
        Struct,   // 嵌套结构体
        Unknown
    };

    template<typename T>
    EShaderParamType GetShaderParamType() {
        if(std::is_same_v<T, float>) return EShaderParamType::Float;
        if(std::is_same_v<T, Float2>) return EShaderParamType::Float2;
        if(std::is_same_v<T, Float3>) return EShaderParamType::Float3;
		if (std::is_same_v<T, Float4>) return EShaderParamType::Float4;
        if(std::is_same_v<T, RenderGraphTexture>) return EShaderParamType::Texture2D;
        if (std::is_same_v<T, RenderGraphBuffer>) return EShaderParamType::Struct;
        return EShaderParamType::Unknown;
    }

    // 对齐规则（简化，UE 实际有更复杂的 D3D/Vulkan 规则）
    inline size_t GetAlignment(EShaderParamType type) {
        switch (type) {
        case EShaderParamType::Float:    return 4;
        case EShaderParamType::Float2:   return 8;
        case EShaderParamType::Float3:   return 16; // std140 风格对齐
        case EShaderParamType::Float4:   return 16;
        case EShaderParamType::Matrix4x4:return 16;
        default: return 16;
        }
    }

    // 单个参数的元信息
    struct ShaderMetaData {
        std::string Name;
        EShaderParamType Type;
        int BindSlot = -1;    // 对于 CBV/SRV/UAV/Texture/Sampler 有效
        size_t Offset = 0;    // 在常量缓冲中的偏移
        size_t Size = 0;      // 占用大小
        std::vector<ShaderMetaData> Nested; // 嵌套结构体
    };



// -------------------------------------------------------------------------------------------------
//  Shader Parameter Base Class
// -------------------------------------------------------------------------------------------------
class ShaderParameter
{
public:
    virtual ~ShaderParameter() = default;
    ShaderParameter(const std::string& name = "") : Name(name) {}
    // Get the name of the parameter
    const std::string& GetName() const { return Name; }

private:
    std::string Name; // Parameter name

};


// -------------------------------------------------------------------------------------------------
//  Typed Shader Parameter Template Class
// -------------------------------------------------------------------------------------------------
template<class T>
struct TypedShaderParameter : public ShaderParameter
{

    explicit TypedShaderParameter(const std::string& name = "", const T& value = {})
        : ShaderParameter(name), Value(value) {}
    T Value; // Stored value of the parameter
    static std::vector<ShaderMetaData> GetMetaDatas() { return {}; }
};


TypedShaderParameter<float> FloatParameter;



// -------------------------------------------------------------------------------------------------
//  Resource Shader Parameters (Texture and Buffer)
// -------------------------------------------------------------------------------------------------
class TextureParameter : public ShaderParameter
{
public:
    explicit TextureParameter(const std::string& name = "", RenderGraphTexture* textureResource = nullptr)
        : ShaderParameter(name), Value(textureResource) {}

    RenderGraphTexture* Value;
    static std::vector<ShaderMetaData> GetMetaDatas() { return {}; }
};

class UniformBufferParameter : public ShaderParameter
{
public:
    explicit UniformBufferParameter(const std::string& name = "", RenderGraphBuffer* bufferResource = nullptr)
        : ShaderParameter(name), Value(bufferResource) {}

    RenderGraphBuffer* Value; // Pointer to the RenderGraphBuffer resource
    static std::vector<ShaderMetaData> GetMetaDatas() { return {}; }
};


// -------------------------------------------------------------------------------------------------
//  Shader Macros for Simplified Shader Definitions
// -------------------------------------------------------------------------------------------------
struct ShaderParameters;

// Begin shader parameter struct
#define BEGIN_SHADER_PARAMETER_STRUCT(StructClass) struct StructClass{ \
    struct FirstIdType{}; \
    using PrevMemberIdType = FirstIdType; \
    using FuncPtr = void*;\
    typedef FuncPtr(*MemberFuncType)(PrevMemberIdType, std::vector<ShaderMetaData>*);\
    static FuncPtr sAppendMemberGetPrev(PrevMemberIdType, std::vector<ShaderMetaData>*) \
		{ \
			return nullptr; \
		} \
	typedef PrevMemberIdType
// End shader parameter struct
#define END_SHADER_PARAMETER_STRUCT() \
    LastIdType;\
    std::vector<ShaderMetaData> GetMetaDatas() \
    {\
        std::vector<ShaderMetaData> res;\
        FuncPtr(*PrevFunc)(LastIdType, std::vector<ShaderMetaData>*);            \
        PrevFunc = sAppendMemberGetPrev; \
        FuncPtr func = (FuncPtr)PrevFunc; \
        do{\
            func = reinterpret_cast<MemberFuncType>(func)(LastIdType(), &res);\
	    } while (func != nullptr); \
        return res;\
    }\
};

#define SHADER_PARAMETER_INTERNAL(CurMemberIdType,PreMemberIdType,ClassType,MemberName,ShaderParameterClass)\
    struct CurMemberIdType : PreMemberIdType{};\
    static FuncPtr sAppendMemberGetPrev(CurMemberIdType, std::vector<ShaderMetaData>* metaDatas) \
		{ \
            ShaderMetaData a{#MemberName,GetShaderParamType<ClassType>(),-1,0,0,ShaderParameterClass::GetMetaDatas()};\
            metaDatas->push_back(a);\
			FuncPtr(*PrevFunc)(PreMemberIdType, std::vector<ShaderMetaData>*); \
			PrevFunc = sAppendMemberGetPrev; \
            return (FuncPtr)PrevFunc; \
		} \
	typedef CurMemberIdType

// Define a texture parameter
#define SHADER_PARAMETER(ClassType,Name) \
 PrevType##Name;\
TypedShaderParameter<ClassType> Name;\
SHADER_PARAMETER_INTERNAL( CurMember##Name,PrevType##Name,ClassType,Name,TypedShaderParameter<ClassType>)

// Define a texture parameter
#define SHADER_PARAMETER_TEXTURE(Name) \
 PrevType##Name;\
TextureParameter Name;\
SHADER_PARAMETER_INTERNAL( CurMember##Name,PrevType##Name,RenderGraphTexture,Name,TextureParameter)

// Define a uniform parameter
#define SHADER_PARAMETER_UNIFORM_BUFFER(Name) \
 PrevType##Name;\
UniformBufferParameter Name;\
SHADER_PARAMETER_INTERNAL( CurMember##Name,PrevType##Name,RenderGraphBuffer,Name,UniformBufferParameter)

BEGIN_SHADER_PARAMETER_STRUCT(A)
    SHADER_PARAMETER(float,T1)
    SHADER_PARAMETER(float,T2)
    SHADER_PARAMETER_TEXTURE(T3)
    SHADER_PARAMETER_UNIFORM_BUFFER(B1)
END_SHADER_PARAMETER_STRUCT();

class ss {
public:
    ss() {
		auto a = A().GetMetaDatas();
        for (int i = 0; i < a.size(); i++) {
            printf("%s\n", a[i].Name.c_str());
        }

    }


};
ss s;
using ShaderParameterSP = std::shared_ptr<ShaderParameter>;






} // namespace WR::RenderCore