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

namespace RenderCore {
    using namespace Core;
    // shader paramter meta data
    struct RENDERCORE_API ShaderMetaData {
        EShaderUniformBaseType Type;
        int numRow = 1;
        int numColumn = 1;
        int BindSlot = -1;    // ���� CBV/SRV/UAV/Texture/Sampler ��Ч
        size_t Offset = 0;    // �ڳ��������е�ƫ��
        size_t Size = 0;      // ռ�ô�С
        std::vector<ShaderMetaData> Nested; // Ƕ�׽ṹ��
    };

// -------------------------------------------------------------------------------------------------
//  Shader Parameter Base Class
// -------------------------------------------------------------------------------------------------
class RENDERCORE_API ShaderParameter
{
public:
    virtual ~ShaderParameter() = default;
    ShaderParameter(const std::string& name = "") : Name(name) {}
    // Get the name of the parameter
    const std::string& GetName() const { return Name; }
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData metadata{};
        return metadata; 
    }
    virtual size_t GetParameterSize() const { return 0; };
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
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData metadata{};
        return metadata;
    }
};
// Float
template<>
struct RENDERCORE_API TypedShaderParameter<float> : public ShaderParameter
{
    explicit TypedShaderParameter(const std::string& name = "", const float& value = 0.0f)
        : ShaderParameter(name), Value(value) {
    }

    float Value;

    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData meta{
            EShaderUniformBaseType::Float32,
            1, 1,          // rows, cols
            -1,            // BindSlot
            0,             // Offset (稍后根据 struct 计算)
            sizeof(TypedShaderParameter<float>), // Size
        };
        return meta;
    }
};

// -------------------------------------------------------------------------------------------------
//  Resource Shader Parameters (Texture and Buffer)
// -------------------------------------------------------------------------------------------------

class RENDERCORE_API TextureParameter : public ShaderParameter
{
public:
    explicit TextureParameter(const std::string& name = "")
        : ShaderParameter(name) {
    }

    RenderGraphTextureSP Value;
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData meta{
            EShaderUniformBaseType::Texture,
            1, 1,          // rows, cols
            -1,            // BindSlot
            0,             // Offset (稍后根据 struct 计算)
            sizeof(RenderGraphTextureSP), // Size
        };
        return meta;
    }
};

class RENDERCORE_API TextureSRVParameter : public ShaderParameter
{
public:
    explicit TextureSRVParameter(const std::string& name = "", RenderGraphTextureSRVSP textureResource = nullptr)
        : ShaderParameter(name), Value(textureResource) {}

    RenderGraphTextureSRVSP Value;
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData meta{
            EShaderUniformBaseType::Texture_SRV,
            1, 1,          // rows, cols
            -1,            // BindSlot
            0,             // Offset (稍后根据 struct 计算)
            sizeof(TextureSRVParameter), // Size
        };
        return meta;
    }
};

class RENDERCORE_API TextureUAVParameter : public ShaderParameter
{
public:
    explicit TextureUAVParameter(const std::string& name = "", RenderGraphTextureUAVSP textureResource = nullptr)
        : ShaderParameter(name), Value(textureResource) {}

    RenderGraphTextureUAVSP Value;
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData meta{
            EShaderUniformBaseType::Texture_UAV,
            1, 1,          // rows, cols
            -1,            // BindSlot
            0,             // Offset (稍后根据 struct 计算)
            sizeof(TextureUAVParameter), // Size
        };
        return meta;
    }
};

class RENDERCORE_API UniformBufferParameter : public ShaderParameter
{
public:
    explicit UniformBufferParameter(const std::string& name = "")
        : ShaderParameter(name) {
    }

    RenderGraphBufferSP Value; // Pointer to the RenderGraphBuffer resource
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData meta{
            EShaderUniformBaseType::Buffer,
            1, 1,          // rows, cols
            -1,            // BindSlot
            0,             // Offset (稍后根据 struct 计算)
            sizeof(UniformBufferParameter), // Size
        };
        return meta;
    }
};

class RENDERCORE_API UniformBufferSRVParameter : public ShaderParameter
{
public:
    explicit UniformBufferSRVParameter(const std::string& name = "", RenderGraphBufferSRVSP bufferResource = nullptr)
        : ShaderParameter(name), Value(bufferResource) {}

    RenderGraphBufferSRVSP Value; // Pointer to the RenderGraphBuffer resource
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData meta{
            EShaderUniformBaseType::Buffer_SRV,
            1, 1,          // rows, cols
            -1,            // BindSlot
            0,             // Offset (稍后根据 struct 计算)
            sizeof(UniformBufferSRVParameter), // Size
        };
        return meta;
    }
};

class RENDERCORE_API UniformBufferUAVParameter : public ShaderParameter
{
public:
    explicit UniformBufferUAVParameter(const std::string& name = "", RenderGraphBufferUAVSP bufferResource = nullptr)
        : ShaderParameter(name), Value(bufferResource) {}

    RenderGraphBufferUAVSP Value; // Pointer to the RenderGraphBuffer resource
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData meta{
            EShaderUniformBaseType::Buffer_UAV,
            1, 1,          // rows, cols
            -1,            // BindSlot
            0,             // Offset (稍后根据 struct 计算)
            sizeof(UniformBufferUAVParameter), // Size
        };
        return meta;
    }
};

struct RENDERCORE_API RenderTargetBinding {
    RenderGraphTextureSP Texture;
    int mipLevel = 0;
    int arraySlice = 0;
};
struct RENDERCORE_API RenderTargetBindingParameter : public ShaderParameter
{
public:
    std::array<RenderTargetBinding, 8> ColorTargets;
    RenderGraphTextureSP DepthStencilTarget;
    static ShaderMetaData& GetMetaData() {
        static ShaderMetaData meta{
            EShaderUniformBaseType::ColorBindings,
            1, 1,          // rows, cols
            -1,            // BindSlot
            0,             // Offset (稍后根据 struct 计算)
            sizeof(RenderTargetBindingParameter), // Size
        };
        return meta;
    }
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
#define END_SHADER_PARAMETER_STRUCT(StructClass) \
    LastIdType;\
    static ShaderMetaData& GetMetaData() \
    {\
        ShaderMetaData metadata;\
        metadata.Size = sizeof(StructClass);\
        FuncPtr(*PrevFunc)(LastIdType, std::vector<ShaderMetaData>*);            \
        PrevFunc = sAppendMemberGetPrev; \
        FuncPtr func = (FuncPtr)PrevFunc; \
        do{\
            func = reinterpret_cast<MemberFuncType>(func)(LastIdType(), &metadata.Nested);\
	    } while (func != nullptr); \
        return metadata;\
    }\
};

#define SHADER_PARAMETER_INTERNAL(CurMemberIdType,PreMemberIdType,ClassType,MemberName,ShaderParameterClass)\
    struct CurMemberIdType : PreMemberIdType{};\
    static FuncPtr sAppendMemberGetPrev(CurMemberIdType, std::vector<ShaderMetaData>* metaDatas) \
		{ \
            metaDatas->push_back(ShaderParameterClass::GetMetaData());\
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

#define SHADER_PARAMETER_TEXTURE_SRV(Name) \
 PrevType##Name;\
TextureSRVParameter Name;\
SHADER_PARAMETER_INTERNAL( CurMember##Name,PrevType##Name,RenderGraphTexture,Name,TextureSRVParameter)

#define SHADER_PARAMETER_TEXTURE_UAV(Name) \
 PrevType##Name;\
TextureUAVParameter Name;\
SHADER_PARAMETER_INTERNAL( CurMember##Name,PrevType##Name,RenderGraphTexture,Name,TextureUAVParameter)


// Define a uniform parameter
#define SHADER_PARAMETER_UNIFORM_BUFFER(Name) \
 PrevType##Name;\
UniformBufferParameter Name;\
SHADER_PARAMETER_INTERNAL( CurMember##Name,PrevType##Name,RenderGraphBuffer,Name,UniformBufferParameter)

#define SHADER_PARAMETER_UNIFORM_BUFFER_SRV(Name) \
 PrevType##Name;\
UniformBufferSRVParameter Name;\
SHADER_PARAMETER_INTERNAL( CurMember##Name,PrevType##Name,RenderGraphBuffer,Name,UniformBufferSRVParameter)

#define SHADER_PARAMETER_UNIFORM_BUFFER_UAV(Name) \
 PrevType##Name;\
UniformBufferUAVParameter Name;\
SHADER_PARAMETER_INTERNAL( CurMember##Name,PrevType##Name,RenderGraphBuffer,Name,UniformBufferUAVParameter)

#define SHADER_PARAMETER_RENDER_TARGETS() \
 PrevType##RenderTargets;\
RenderTargetBindingParameter RenderTargets;\
SHADER_PARAMETER_INTERNAL( CurMember##RenderTargets,PrevType##RenderTargets,RenderTargetBindingParameter,RenderTargets,RenderTargetBindingParameter)



BEGIN_SHADER_PARAMETER_STRUCT(A)
    SHADER_PARAMETER(float,T1)
    SHADER_PARAMETER(float,T2)
    SHADER_PARAMETER_TEXTURE_SRV(T3)
    SHADER_PARAMETER_UNIFORM_BUFFER_SRV(B1)
    SHADER_PARAMETER_RENDER_TARGETS()
END_SHADER_PARAMETER_STRUCT(A);

//class ss {
//public:
//    ss() {
//		auto a = A().GetMetaDatas();
//        for (int i = 0; i < a.size(); i++) {
//            printf("%s\n", a[i].Name.c_str());
//        }
//
//    }
//
//
//};
//ss s;
using ShaderParameterSP = std::shared_ptr<ShaderParameter>;


struct RENDERCORE_API ShaderParameterInstance
{
    EShaderUniformBaseType Type;
    void* Ptr = nullptr;   // 指向实际参数内存
    size_t Size = 0;
};

class RENDERCORE_API ShaderParameterReader
{
public:
    ShaderParameterReader(void* rawParameters, const ShaderMetaData& metaData)
        : RawParameters(reinterpret_cast<uint8_t*>(rawParameters))
        , ParameterMetaData(metaData)
    {
        Reset();
    }

    void Reset()
    {
        Stack.clear();
        IndexStack.clear();
        Stack.push_back(&ParameterMetaData);
        IndexStack.push_back(0);
    }

    // 返回下一个非 struct 参数
    bool ReadNext(ShaderParameterInstance& outParam)
    {
        while (!Stack.empty())
        {
            const ShaderMetaData* currentMeta = Stack.back();
            size_t& currentIndex = IndexStack.back();

            // 遍历 struct 的 Nested
            if (currentIndex < currentMeta->Nested.size())
            {
                const ShaderMetaData& memberMeta = currentMeta->Nested[currentIndex++];

                if (memberMeta.Type == EShaderUniformBaseType::Struct)
                {
                    // 进入 struct，递归展开
                    Stack.push_back(&memberMeta);
                    IndexStack.push_back(0);
                    continue;
                }

                // 非 struct，返回参数
                outParam.Type = memberMeta.Type;
                outParam.Ptr = RawParameters + memberMeta.Offset;
                outParam.Size = memberMeta.Size;
                return true;
            }
            else
            {
                // struct 遍历完成，弹出
                Stack.pop_back();
                IndexStack.pop_back();
            }
        }

        // 没有更多参数
        return false;
    }

    std::map<std::string,ShaderParameter*> GetAllParameterMaps() {
        std::map<std::string, ShaderParameter*> result;
        ShaderParameterInstance param;
        while (ReadNext(param)) {
            auto parameter = static_cast<ShaderParameter*>(param.Ptr);
            auto name = parameter->GetName();
            result[name] = parameter;
        }
        return result;
    }

private:
    uint8_t* RawParameters = nullptr;
    const ShaderMetaData& ParameterMetaData;

    // 用栈保存当前递归 struct 的状态
    std::vector<const ShaderMetaData*> Stack;
    std::vector<size_t> IndexStack;
};


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
    ShaderMetaData ContentMetaData;
    // Set shader parameters into command list for the given shader
    void SetShaderParameters(RHI::RHICommandList& cmdList, const ShaderSP& shader, ShaderParameterBindingInfo* bindingInfo) const
    {
        if (!Paramters || !shader)
            return;

        if (!bindingInfo)
            return;

        // Create batched shader parameters
        RHI::RHIBatchedShaderParameters batchedParams;

        auto allShaderParameters = ShaderParameterReader(Paramters, ContentMetaData).GetAllParameterMaps();

        // Traverse ContentMetaData and match with binding info
        for (const auto& paramInfo : allShaderParameters)
        {
            const ShaderParameterBinding* binding = bindingInfo->GetParameterBinding(paramInfo.first);
            if (!binding)
                continue; // Skip unmatched parameters
            auto parameter = paramInfo.second;

            //
            RHIShaderParameterDesc desc;
            //convert parameter to rhi shader parameter

            
            batchedParams.Parameters.emplace_back(desc);
        }

        // Set the batched parameters into the command list
        cmdList.SetBatchedShaderParameters(shader->GetRHIShader(), batchedParams);
    }
};


} // namespace WR::RenderCore