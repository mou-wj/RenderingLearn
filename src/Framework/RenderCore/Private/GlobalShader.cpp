#include "GlobalShader.h"
#include "ShaderCompiler.h"
#include "RHIApi.h"
#include <functional>
#include <sstream>

namespace RenderCore{

    // static members

    GlobalShaderMap GlobalShaderMapInstance;
    GlobalShaderMap& RenderCore::GetGlobalShaderMap()
    {
        GlobalShaderMapInstance.Initialize();
        return GlobalShaderMapInstance;
    }

    ShaderSP GlobalShaderMap::GetShader(ShaderType* shaderType, ShaderPermutationId id)
    {
        auto it = ShaderMap.find(shaderType);
        if (it != ShaderMap.end()) {
            auto shaderIt = it->second.find(id);
            if (shaderIt != it->second.end()) {
                return shaderIt->second;
            }
        }
        return nullptr;
    }

    // Helper: build a simple stable key string for a ShaderType + platform + perm
    static std::string BuildShaderUniqueString(const ShaderType* st, RHI::ERHIShaderPlatform platform, ShaderPermutationId perm)
    {
        std::ostringstream oss;
        oss << st->Name << "|" << st->SourceFile << "|" << st->EntryPoint << "|" << static_cast<int>(platform) << "|" << perm;
        return oss.str();
    }

    bool GlobalShaderMap::Initialize()
    {
        if (IsInitialized) {
            return true;
        }

        // Iterate registered global shader types and compile / instantiate them
        const auto& globalTypes = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::Global];
        for (const auto& kv : globalTypes)
        {
            ShaderType* st = kv.second;
            if (!st) continue;

            // set permutation id = 0 (global shaders use default permutation)
            ShaderPermutationId permId = 0;

            // Determine current platform (extern from ShaderLibrary.h)
            RHI::ERHIShaderPlatform platform = GShaderPlatform;

            // Build a ShaderKey (local representation)
            ShaderKey key(st, platform, permId, nullptr);

            // Decide whether to compile this permutation
            ShaderPermutationParameters permParams;
            permParams.PermutationId = permId;
            bool bShouldCompile = true;
            if (st->ShouldCompilePermutation)
            {
                bShouldCompile = st->ShouldCompilePermutation(permParams);
            }
            if (!bShouldCompile)
            {
                continue;
            }

            // Allow shader to modify compiler environment
            ShaderCompilerEnvironment env;
            if (st->ModifyCompilationEnvironment)
            {
                //st->ModifyCompilationEnvironment(env);
            }

            // Create a simple hashed key for the RHI layer
            std::string unique = BuildShaderUniqueString(st, platform, permId);
            // Assuming RHI provides a ShaderKeyHash type with constructor from uint64_t or similar.
            // We produce a uint64_t hash and wrap it in RHI::ShaderKeyHash if available.
            uint64_t hash64 = std::hash<std::string>()(unique);
            RHI::ShaderKeyHash rhiHash;
            rhiHash = hash64; // requires RHI::ShaderKeyHash to have member 'Hash' (adapt if different)

            // Compile/Create RHI shader using ShaderLibrary according to frequency
            RHI::RHIShaderSP rhiShader;
            switch (st->Frequency)
            {
            case RHI::ERHIShaderFrequency::Vertex:

                break;
            case RHI::ERHIShaderFrequency::Fragment:
                
                break;
            case RHI::ERHIShaderFrequency::Compute:
                
                break;
            case RHI::ERHIShaderFrequency::Geometry:
                
                break;
            case RHI::ERHIShaderFrequency::TessControl:

                break;
            case RHI::ERHIShaderFrequency::TessEvaluation:

                break;
            default:
                rhiShader = nullptr;
                break;
            }

            if (!rhiShader)
            {
                // creation/compilation failed for this shader; skip storing
                continue;
            }

            // Create a generic Shader instance and attach the RHI shader
            // Use the ShaderType's name/source to construct the Shader
            ShaderSP shaderInstance ;
            shaderInstance->SetRHIShader(rhiShader);

            // store into map
            ShaderMap[st][permId] = shaderInstance;
        }

        IsInitialized = true;
        return true;
    }

} // namespace RenderCore

