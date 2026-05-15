#pragma once

#include "RHIResource.h" // For RHIShaderSP, ERHIResourceType
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
    struct ShaderParameterAllocation
    {
        uint16_t BufferIndex = 0;   // 所属 UniformBuffer 索引
        uint16_t BaseIndex = 0;     // GPU binding slot
        uint16_t Size = 0;          // 字节大小
        EShaderParameterType Type = EShaderParameterType::Num;
        mutable bool bBound = false;

        ShaderParameterAllocation() = default;

        ShaderParameterAllocation(uint16_t InBufferIndex, uint16_t InBaseIndex, uint16_t InSize, EShaderParameterType InType)
            : BufferIndex(InBufferIndex)
            , BaseIndex(InBaseIndex)
            , Size(InSize)
            , Type(InType)
        {
        }
    };

    // ShaderParameterMap：管理 Shader 参数名 → 分配信息
    class ShaderParameterAllocationMap
    {
    public:
        ShaderParameterAllocationMap() = default;

        // 查找参数
        std::optional<ShaderParameterAllocation> FindParameterAllocation(const std::string& Name) const
        {
            auto it = ParameterMap.find(Name);
            if (it != ParameterMap.end())
                return it->second;
            return std::nullopt;
        }

        // 添加参数
        void AddParameterAllocation(const std::string& Name, uint16_t BufferIndex, uint16_t BaseIndex, uint16_t Size, EShaderParameterType Type)
        {
            ParameterMap[Name] = ShaderParameterAllocation(BufferIndex, BaseIndex, Size, Type);
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

        const std::unordered_map<std::string, ShaderParameterAllocation>& GetParameterMap() const {
            return ParameterMap;
        }

    private:
        std::unordered_map<std::string, ShaderParameterAllocation> ParameterMap;
    };




    // 上层 RenderCore 的绑定信息
    struct RENDERCORE_API ShaderParameterBindingInfo
    {
        // --------- RenderCore 层的 Shader Parameter Binding ---------
        struct RENDERCORE_API ShaderUniformBinding
        {
            EShaderParameterBaseType BaseType; // Float, Int, Bool 等基础类型
            uint32_t BufferIndex;              // 所属 UniformBuffer 索引
            uint32_t BaseIndex;                // GPU binding 中的索引
            uint32_t Offset;                 // 在统一 CPU buffer 中的偏移
            uint32_t Size;                   // 字节大小
        };

        struct RENDERCORE_API ShaderResourceBinding
        {
            EShaderParameterBaseType BaseType; // Texture / Buffer / UAV / Sampler
            uint16_t BindSlot;               // GPU绑定槽
            uint16_t ArraySize = 1;          // 支持数组
            uint32_t Offset;
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
    class Shader;
    using ModifyCompilationEnvironmentFuncType = std::function< void(const ShaderPermutationParameters&, ShaderCompilerEnvironment&)>;
    using ShouldCompilePermutationFuncType = std::function< bool(const ShaderPermutationParameters&)>;
    struct ShaderCompiledInitializer;
    using ConstructCompiledFuncType = std::function< Shader* (const ShaderCompiledInitializer&)>;
    struct ShaderCompiledInitializer
    {
        const ShaderType* Type;
        const std::vector<char>& Code;
        const ShaderParameterAllocationMap& ParameterMap;
        uint32_t PermutationId;
        ShaderCompiledInitializer(const ShaderType* Type,
            const std::vector<char>& Code,
            const ShaderParameterAllocationMap& ParameterMap,
            uint32_t PermutationId):Type(Type) , Code(Code), ParameterMap(ParameterMap), PermutationId(PermutationId) {}
    };

    // ShaderType ��
    struct RENDERCORE_API ShaderType
    {
public:
    enum class EShaderTypeFlag : uint8_t
    {
        Global = 0,
        Material = 1 << 0, // 是否为全局 Shader（不依赖 VertexFactory）
        MeshMaterial = 1 << 1, // 是否为 MeshMaterial Shader（依赖 VertexFactory）
        // Future flags can be added here (e.g., for editor-only shaders, mobile shaders, etc.)
    };

    std::string Name;             // Shader 名称，如 BasePassVS
    std::string SourceFile;       // USF 源文件路径
    std::string EntryPoint;       // 入口函数名
    RHI::ERHIShaderFrequency Frequency;   // VS / PS / CS

    ModifyCompilationEnvironmentFuncType ModifyCompilationEnvironment;
    ShouldCompilePermutationFuncType ShouldCompilePermutation;
    ConstructCompiledFuncType ConstructCompiled;
    int32_t TotalPermutationCount = 1;
    const ShaderParametersMetadata* RootParametersMetadata = nullptr;
    EShaderTypeFlag Flag = EShaderTypeFlag::Global;

    ShaderType(
        const std::string& InName,
        const std::string& InSourceFile,
        const std::string& InEntryPoint,
        RHI::ERHIShaderFrequency InFrequency,
        ModifyCompilationEnvironmentFuncType InModifyCompilationEnvironment,
        ShouldCompilePermutationFuncType InShouldCompilePermutation,
        ConstructCompiledFuncType InConstructCompiled,
        int32_t InTotalPermutationCount,
        const ShaderParametersMetadata* InRootParametersMetadata,
        EShaderTypeFlag InFlag
    )
        :
        Name(InName),
        SourceFile(InSourceFile),
        EntryPoint(InEntryPoint),
        Frequency(InFrequency),
        ModifyCompilationEnvironment(InModifyCompilationEnvironment),
        ShouldCompilePermutation(InShouldCompilePermutation),
        ConstructCompiled(InConstructCompiled),
        TotalPermutationCount(InTotalPermutationCount),
		RootParametersMetadata(InRootParametersMetadata),
        Flag(InFlag)
    {
        Register(this);
    }

    // 全局注册表相关静态方法
    // flag -> (name -> ShaderType*)
    static std::unordered_map<EShaderTypeFlag, std::unordered_map<std::string, ShaderType*>>& GetRegisterMap() {
        static std::unordered_map<EShaderTypeFlag, std::unordered_map<std::string, ShaderType*>> Map;
        return Map;
    }
    // 注册到指定 flag 分组
    static void Register(const std::string& name, ShaderType* type, EShaderTypeFlag flag) {
        type->Flag = flag;
        GetRegisterMap()[flag][name] = type;
    }
    // 按 flag+name 查找
    static ShaderType* Find(const std::string& name, EShaderTypeFlag flag) {
        auto& map = GetRegisterMap();
        auto fit = map.find(flag);
        if (fit != map.end()) {
            auto nit = fit->second.find(name);
            if (nit != fit->second.end())
                return nit->second;
        }
        return nullptr;
    }
    // 获取某 flag 下所有 ShaderType
    static std::vector<ShaderType*> GetAllOfFlag(EShaderTypeFlag flag) {
        std::vector<ShaderType*> result;
        auto& map = GetRegisterMap();
        auto fit = map.find(flag);
        if (fit != map.end()) {
            for (const auto& kv : fit->second) {
                result.push_back(kv.second);
            }
        }
        return result;
    }
    private:
        static void Register(ShaderType* Type)
        {
            GetRegisterMap()[Type->Flag][Type->Name] = Type;
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
    using PermutationDomain = ShaderPermutationDomain<>;
    // Construction/Destruction
    Shader(const ShaderCompiledInitializer& initializer);
    virtual ~Shader();

    // Accessors
    const std::string& GetName() const { return Name; }
    const std::vector<char>& GetShaderSourceCode() const { return ShaderSourceCode; }
    ERHIShaderFrequency GetShaderFrequency() const { return ShaderType; }

    // RHI Resource
    RHIShaderSP GetRHIShader() const { return RHIShader; }
    void SetRHIShader(RHIShaderSP shader) { RHIShader = shader; }

    // Compilation (Called by the RenderGraphBuilder)
    bool Compile();



    static void ModifyShaderCompilerEnvironment(const ShaderPermutationParameters& ,ShaderCompilerEnvironment& Env) {}

    static bool ShouldCompilePermutation(const ShaderPermutationParameters& param) { return true; }

    const ShaderParameterBindingInfo& GetParameterBindings() const { return Bindings; }

protected:
    void InitShaderBindings(const ShaderParametersMetadata* Metadata,
        const ShaderParameterAllocationMap& InParameterMap);
    void ProcessMetadataRecursive(
        const ShaderParametersMetadata& Metadata,
        const std::string& Prefix,
        const ShaderParameterAllocationMap& ParameterMap,
        ShaderParameterBindingInfo& OutBindings);

    void InitShaderRHI(ERHIShaderFrequency frequency, const std::vector<char>& shaderSourceCode);
    // Shader Name (for debugging and identification)
    std::string Name;

    // Shader Source Code
    std::vector<char> ShaderSourceCode;

    // Shader Type (Vertex, Fragment, Compute, etc.)
    ERHIShaderFrequency ShaderType;

    // RHI Shader Resource
    RHIShaderSP RHIShader;

    ShaderParameterBindingInfo Bindings;

public:

};


using ShaderSP = std::shared_ptr<Shader>;
} // namespace RenderCore


#define DECLARE_SHADER_TYPE(ShaderClass) \
public: \
    static ShaderMetaType StaticType; \
    static RenderCore::ShaderType* GetStaticType() \
    { \
        return &StaticType; \
    }


// 注册宏：支持传入 flag，自动注册到对应分组
#define IMPLEMENT_SHADER_TYPE( \
    ShaderClass, \
    ShaderName, \
    ShaderPath, \
    EntryPoint, \
    Frequency\
) \
    ShaderClass::ShaderMetaType ShaderClass::StaticType( \
        ShaderName, \
        ShaderPath, \
        EntryPoint, \
        Frequency, \
        &ShaderClass::ModifyShaderCompilerEnvironment, \
        &ShaderClass::ShouldCompilePermutation, \
        [](const RenderCore::ShaderCompiledInitializer& initializer){return new ShaderClass(static_cast<const ShaderClass::ShaderMetaType::ShaderCompiledInitializer&>(initializer));}, \
        ShaderClass::PermutationDomain::TotalCount, \
        ShaderClass::GetShaderParameterMetadata()\
    );

