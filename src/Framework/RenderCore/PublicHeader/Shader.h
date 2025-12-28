#pragma once

#include "RHIResource.h" // For RHIShaderSP, ERHIResourceType
#include "ShaderLibrary.h"
#include "ShaderCore.h"
#include "VertexFactory.h"
#include <vector>
#include <string>
#include <unordered_map>

using namespace RHI;

namespace RenderCore {

    struct RENDERCORE_API ShaderParameterBinding{
            EShaderUniformBaseType ParameterBaseType;
            uint32_t NumRow;
            int32_t NumColumn;  
            uint32_t BindSlot;      
    };
    struct RENDERCORE_API ShaderParameterBindingInfo{
        std::unordered_map<std::string, ShaderParameterBinding> ParameterBindings;

        void AddParameterBinding(const std::string& name, const ShaderParameterBinding& binding)
        {
            ParameterBindings[name] = binding;
        }

        const ShaderParameterBinding* GetParameterBinding(const std::string& name) const
        {
            auto it = ParameterBindings.find(name);
            if (it != ParameterBindings.end())
            {
                return &it->second;
            }
            return nullptr;
        }

    };

    // ShaderType ��
    struct RENDERCORE_API ShaderType
    {
    public:
        std::string Name;             // Shader���ƣ��� BasePassVS
        std::string SourceFile;       // USF Դ�ļ�·��
        std::string EntryPoint;       // ������ں���
        RHI::ERHIShaderType Frequency;   // VS / PS / CS
        ShaderParameterBindingInfo ParameterBindingInfo;
		std::function< void(ShaderCompilerEnvironment&)> ModifyCompilationEnvironment;
        std::function< bool(const ShaderPermutationParameters&)> ShouldCompilePermutation;

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
    Shader(const std::string& name, const std::vector<char>& shaderSourceCode, ERHIShaderType shaderType);
    Shader(const std::string& name, const std::string& shaderSourceCodePath, ERHIShaderType shaderType);
    virtual ~Shader();

    // Accessors
    const std::string& GetName() const { return Name; }
    const std::vector<char>& GetShaderSourceCode() const { return ShaderSourceCode; }
    ERHIShaderType GetShaderType() const { return ShaderType; }

    // RHI Resource
    RHIShaderSP GetRHIShader() const { return RHIShader; }
    void SetRHIShader(RHIShaderSP shader) { RHIShader = shader; }

    // Compilation (Called by the RenderGraphBuilder)
    bool Compile();

    virtual const ShaderParameterBindingInfo& GetShaderParameterBindingInfo() const {
        static ShaderParameterBindingInfo EmptyBindingInfo;
        return EmptyBindingInfo;
    }

    static void ModifyShaderCompilerEnvironment(ShaderCompilerEnvironment& Env) {}

    static bool ShouldCompilePermutation(const ShaderPermutationParameters& param) { return true; }

protected:
    virtual void InitShaderParameters() {}
    // Shader Name (for debugging and identification)
    std::string Name;

    // Shader Source Code
    std::vector<char> ShaderSourceCode;

    // Shader Type (Vertex, Fragment, Compute, etc.)
    ERHIShaderType ShaderType;

    // RHI Shader Resource
    RHIShaderSP RHIShader;

public:

};

using ShaderSP = std::shared_ptr<Shader>;
} // namespace RenderCore