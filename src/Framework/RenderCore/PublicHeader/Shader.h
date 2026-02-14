#pragma once

#include "RHIResource.h" // For RHIShaderSP, ERHIResourceType
#include "ShaderLibrary.h"
#include "ShaderCore.h"
#include "VertexFactory.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

using namespace RHI;

namespace RenderCore {
    class ShaderParametersMetadata;
    enum class EShaderParameterType : uint8_t
    {
        LooseData,
        UniformBuffer,
        Sampler,
        SRV,
        UAV,
        BindlessResourceIndex,
        BindlessSamplerIndex,
        Num
    };

    // 单个参数在 CPU/GPU 中的分配信息
    struct FParameterAllocation
    {
        uint16_t BufferIndex = 0;   // 所属 UniformBuffer 索引
        uint16_t BaseIndex = 0;     // GPU binding slot
        uint16_t Size = 0;          // 字节大小
        EShaderParameterType Type = EShaderParameterType::Num;
        mutable bool bBound = false;

        FParameterAllocation() = default;

        FParameterAllocation(uint16_t InBufferIndex, uint16_t InBaseIndex, uint16_t InSize, EShaderParameterType InType)
            : BufferIndex(InBufferIndex)
            , BaseIndex(InBaseIndex)
            , Size(InSize)
            , Type(InType)
        {
        }
    };

    // ShaderParameterMap：管理 Shader 参数名 → 分配信息
    class ShaderParameterMap
    {
    public:
        ShaderParameterMap() = default;

        // 查找参数
        std::optional<FParameterAllocation> FindParameterAllocation(const std::string& Name) const
        {
            auto it = ParameterMap.find(Name);
            if (it != ParameterMap.end())
                return it->second;
            return std::nullopt;
        }

        // 添加参数
        void AddParameterAllocation(const std::string& Name, uint16_t BufferIndex, uint16_t BaseIndex, uint16_t Size, EShaderParameterType Type)
        {
            ParameterMap[Name] = FParameterAllocation(BufferIndex, BaseIndex, Size, Type);
        }

        // 移除参数
        void RemoveParameterAllocation(const std::string& Name)
        {
            ParameterMap.erase(Name);
        }

        // 判断参数是否存在
        bool ContainsParameterAllocation(const std::string& Name) const
        {
            return ParameterMap.find(Name) != ParameterMap.end();
        }

        // 获取同类型参数列表
        std::vector<std::string> GetAllParameterNamesOfType(EShaderParameterType Type) const
        {
            std::vector<std::string> Names;
            for (const auto& kv : ParameterMap)
            {
                if (kv.second.Type == Type)
                    Names.push_back(kv.first);
            }
            return Names;
        }

        // 获取所有参数名
        std::vector<std::string> GetAllParameterNames() const
        {
            std::vector<std::string> Names;
            for (const auto& kv : ParameterMap)
                Names.push_back(kv.first);
            return Names;
        }

    private:
        std::unordered_map<std::string, FParameterAllocation> ParameterMap;
    };




    // 上层 RenderCore 的绑定信息
    struct RENDERCORE_API ShaderParameterBindingInfo
    {
        // --------- RenderCore 层的 Shader Parameter Binding ---------
        struct RENDERCORE_API ShaderUniformBinding
        {
            EShaderUniformBaseType BaseType; // Float, Int, Bool 等基础类型
            uint32_t Offset;                 // 在统一 buffer 中的偏移
            uint32_t Size;                   // 字节大小
        };

        struct RENDERCORE_API ShaderResourceBinding
        {
            EShaderUniformBaseType BaseType; // Texture / Buffer / UAV / Sampler
            uint16_t BindSlot;               // GPU绑定槽
            uint16_t ArraySize = 1;          // 支持数组
        };
        // 名字到 Uniform 参数的映射
        std::unordered_map<std::string, ShaderUniformBinding> UniformBindings;

        // 名字到 Resource 参数的映射
        std::unordered_map<std::string, ShaderResourceBinding> ResourceBindings;

        // 添加 Uniform
        void AddUniformBinding(const std::string& name, const ShaderUniformBinding& binding)
        {
            UniformBindings[name] = binding;
        }

        // 添加 Resource
        void AddResourceBinding(const std::string& name, const ShaderResourceBinding& binding)
        {
            ResourceBindings[name] = binding;
        }

        // 获取 Uniform
        const ShaderUniformBinding* GetUniformBinding(const std::string& name) const
        {
            auto it = UniformBindings.find(name);
            if (it != UniformBindings.end())
                return &it->second;
            return nullptr;
        }

        // 获取 Resource
        const ShaderResourceBinding* GetResourceBinding(const std::string& name) const
        {
            auto it = ResourceBindings.find(name);
            if (it != ResourceBindings.end())
                return &it->second;
            return nullptr;
        }
    };
    struct ShaderType;
    struct ShaderCompiledInitializer
    {
        const ShaderType* Type;
        const std::vector<uint8_t>& Code;
        const ShaderParameterMap& ParameterMap;
        uint32_t PermutationId;
        size_t OutputHash;
    };

    // ShaderType ��
    struct RENDERCORE_API ShaderType
    {
    public:
        std::string Name;             // Shader���ƣ��� BasePassVS
        std::string SourceFile;       // USF Դ�ļ�·��
        std::string EntryPoint;       // ������ں���
        RHI::ERHIShaderFrequency Frequency;   // VS / PS / CS
		std::function< void(ShaderCompilerEnvironment&)> ModifyCompilationEnvironment;
        std::function< bool(const ShaderPermutationParameters&)> ShouldCompilePermutation;
        std::function<RHIShader* (const ShaderCompiledInitializer&)> ConstructCompiled;
        int32_t TotalPermutationCount = 1;
        const ShaderParametersMetadata* RootParametersMetadata = nullptr;
        void operator=(const ShaderType& other) {
            Name = other.Name;
            SourceFile = other.SourceFile;
            EntryPoint = other.EntryPoint;
            Frequency = other.Frequency;
        }
        bool operator==(const ShaderType& other) const {
			return Name == other.Name &&
				SourceFile == other.SourceFile &&
				EntryPoint == other.EntryPoint &&
				Frequency == other.Frequency;
         }
        size_t GetHash() const {
            size_t hash = std::hash<std::string>()(Name);
            hash ^= std::hash<std::string>()(SourceFile) << 1;
            hash ^= std::hash<std::string>()(EntryPoint) << 2;
            hash ^= std::hash<int>()(static_cast<int>(Frequency)) << 3;
            return hash;
        }
    };
    
    class VertexFactoryType;

    // ShaderKey
    struct RENDERCORE_API ShaderKey
    {
        // Shader ����ָ��
        ShaderType* ShaderTypeP;

        // Shader ����ƽ̨
        RHI::ERHIShaderPlatform Platform;

        // Shader permutation ID�������/���Ա��壩
        ShaderPermutationId PermutationId;

        // VertexFactory ����ָ�루�� VertexShader �� MeshShader ��Ч��
        VertexFactoryType* VFType = nullptr;

        // ���캯��
        ShaderKey(
            ShaderType* ShaderType,
            RHI::ERHIShaderPlatform platform,
            ShaderPermutationId permId,
            VertexFactoryType* vfType = nullptr)
            : ShaderTypeP(ShaderType)
            , Platform(platform)
            , PermutationId(permId)
            , VFType(vfType)
        {
        }

        // �Ƚ������������ map / set ����
        bool operator==(const ShaderKey& other) const
        {
            return ShaderTypeP == other.ShaderTypeP &&
                Platform == other.Platform &&
                PermutationId == other.PermutationId &&
                VFType == other.VFType;
        }

        // ���ɹ�ϣֵ������ unordered_map / unordered_set
        size_t GetHash() const
        {
            size_t hash = std::hash<int>()(static_cast<int>(Platform));
            hash ^= std::hash<int>()(static_cast<int>(Platform)) << 1;
            hash ^= std::hash<ShaderPermutationId>()(PermutationId) << 2;
            hash ^= std::hash<VertexFactoryType*>()(VFType) << 3;
            return hash;
        }
    };


// -------------------------------------------------------------------------------------------------
//  Shader Base Class (for Render Passes) - Mimicking UE's Approach
// -------------------------------------------------------------------------------------------------
class RENDERCORE_API Shader
{
public:
    // Construction/Destruction
    Shader(const std::string& name, const std::vector<char>& shaderSourceCode, ERHIShaderFrequency shaderType);
    Shader(const std::string& name, const std::string& shaderSourceCodePath, ERHIShaderFrequency shaderType);
    virtual ~Shader();

    // Accessors
    const std::string& GetName() const { return Name; }
    const std::vector<char>& GetShaderSourceCode() const { return ShaderSourceCode; }
    ERHIShaderFrequency GetShaderType() const { return ShaderType; }

    // RHI Resource
    RHIShaderSP GetRHIShader() const { return RHIShader; }
    void SetRHIShader(RHIShaderSP shader) { RHIShader = shader; }

    // Compilation (Called by the RenderGraphBuilder)
    bool Compile();

    ShaderParameterBindingInfo Bindings;

    static void ModifyShaderCompilerEnvironment(ShaderCompilerEnvironment& Env) {}

    static bool ShouldCompilePermutation(const ShaderPermutationParameters& param) { return true; }

protected:
    virtual void InitShaderParameters() {}
    // Shader Name (for debugging and identification)
    std::string Name;

    // Shader Source Code
    std::vector<char> ShaderSourceCode;

    // Shader Type (Vertex, Fragment, Compute, etc.)
    ERHIShaderFrequency ShaderType;

    // RHI Shader Resource
    RHIShaderSP RHIShader;

public:

};

using ShaderSP = std::shared_ptr<Shader>;
} // namespace RenderCore