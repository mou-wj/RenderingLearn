#pragma once

#include "RHIResource.h" // For RHIShaderSP, ERHIResourceType
#include "ShaderLibrary.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "ShaderParameter.h"



using namespace RHI;

namespace RenderCore {



// -------------------------------------------------------------------------------------------------
//  Shader Base Class (for Render Passes) - Mimicking UE's Approach
// -------------------------------------------------------------------------------------------------
class Shader
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

};

using ShaderSP = std::shared_ptr<Shader>;

} // namespace WR::RenderCore