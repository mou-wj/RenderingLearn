#include "Shader.h"
#include "RenderGraphBuilder.h"
#include "RHIApi.h"
#include <stdexcept>
#include <iostream>

namespace RenderCore {


Shader::Shader(const ShaderCompiledInitializer& initializer)
{
    Name = initializer.Type->Name;
    ShaderType = initializer.Type->Frequency;
    auto metadata = initializer.Type->RootParametersMetadata;
    if (metadata) {
		InitShaderBindings(metadata, initializer.ParameterMap);
    }
    InitShaderRHI(initializer.Type->Frequency, initializer.Code);
}

// Destructor
Shader::~Shader()
{
    // Cleanup if necessary
    if (RHIShader)
	{
		RHIShader.reset();
		RHIShader = nullptr;
	}
}

// Compilation
bool Shader::Compile()
{
    try
    {
        // Create RHI Shader using RenderGraphBuilder
        //RHIShader = GRHIApi->CreateShader(ShaderSourceCode, ShaderType);
        if (!RHIShader)
        {
            throw std::runtime_error("Failed to create RHI shader for: " + Name);
        }


        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Shader compilation error: " << e.what() << std::endl;
        return false;
    }
}

void Shader::ProcessMetadataRecursive(
    const ShaderParametersMetadata& Metadata,
    const std::string& Prefix,
    const ShaderParameterAllocationMap& ParameterMap,
    ShaderParameterBindingInfo& OutBindings)
{
    for (const auto& Member : Metadata.GetMembers())
    {
        std::string Name = Prefix.empty()
            ? Member.Name
            : Prefix + "_" + Member.Name;

        // =========================
        // 1. Resource（SRV/UAV/Sampler）
        // =========================
        if (Member.IsResource())
        {
            auto Allocation = ParameterMap.FindParameterAllocation(Name);
            if (!Allocation.has_value())
                continue; // shader没用到

            ShaderParameterBindingInfo::ShaderResourceBinding Binding;
            Binding.BaseType = Member.BaseType;
            Binding.BindSlot = Allocation->BaseIndex;
            Binding.ArraySize = (Member.NumElements > 0) ? Member.NumElements : 1;
            Binding.Offset = Member.Offset;

            OutBindings.AddResourceBinding(Name, Binding);
        }
        // =========================
        // 2. Struct（递归展开）
        // =========================
        else if (Member.IsStruct())
        {
            ProcessMetadataRecursive(
                *Member.StructMetadata,
                Name,
                ParameterMap,
                OutBindings);
        }
        // =========================
        // 3. Uniform（关键）
        // =========================
        else
        {
            // Uniform 是走 cbuffer 的

            auto Allocation = ParameterMap.FindParameterAllocation(Name);

            if (!Allocation.has_value())
                continue; // shader没用到

            if (Allocation->Type != EShaderParameterType::LooseData &&
                Allocation->Type != EShaderParameterType::UniformBuffer)
            {
                continue;
            }

            ShaderParameterBindingInfo::ShaderUniformBinding Binding;
            Binding.BaseType = Member.BaseType;
            Binding.BufferIndex = Allocation->BufferIndex;
            Binding.BaseIndex = Allocation->BaseIndex;
            Binding.Offset = Member.Offset;
            Binding.Size = Allocation->Size;

            OutBindings.AddUniformBinding(Name, Binding);
        }
    }
}

void Shader::InitShaderBindings(const ShaderParametersMetadata* Metadata, const ShaderParameterAllocationMap& InParameterMap)
{
    if (!Metadata) return;

    Bindings = {}; // 清空

    if (!Metadata)
        return;

    ProcessMetadataRecursive(
        *Metadata,
        "",                     // prefix
        InParameterMap,
        Bindings);
}

void Shader::InitShaderRHI(ERHIShaderFrequency frequency, const std::vector<char>& shaderSourceCode)
{
    switch (frequency)
    {
    case RHI::ERHIShaderFrequency::Unknown:
        break;
    case RHI::ERHIShaderFrequency::Vertex:
        RHIShader = GRHIApi->CreateVertexShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::Fragment:
        RHIShader = GRHIApi->CreateFragmentShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::Geometry:
        RHIShader = GRHIApi->CreateGeometryShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::Compute:
        RHIShader = GRHIApi->CreateComputeShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::TessControl:
        RHIShader = GRHIApi->CreateTessControlShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::TessEvaluation:
        RHIShader = GRHIApi->CreateTessEvalShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::Mesh:
        RHIShader = GRHIApi->CreateMeshShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::Task:
        RHIShader = GRHIApi->CreateTaskShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::RayGen:
        RHIShader = GRHIApi->CreateRayGenShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::ClosestHit:
        RHIShader = GRHIApi->CreateCloseHitShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::Miss:
        RHIShader = GRHIApi->CreateMissShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::AnyHit:
        RHIShader = GRHIApi->CreateAnyHitShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::Intersection:
        RHIShader = GRHIApi->CreateIntersectionShader(shaderSourceCode);
        break;
    case RHI::ERHIShaderFrequency::Callable:
        RHIShader = GRHIApi->CreateCallableShader(shaderSourceCode);
        break;
    default:
        break;
    }

}


} // namespace RenderCore