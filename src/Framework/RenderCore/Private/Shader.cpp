#include "Shader.h"
#include "RenderGraphBuilder.h"
#include "RHIApi.h"
#include <stdexcept>
#include <iostream>

namespace RenderCore {

// Constructor
Shader::Shader(const std::string& name, const std::vector<char>& shaderSourceCode, ERHIShaderFrequency shaderType)
    : Name(name), ShaderSourceCode(shaderSourceCode), ShaderType(shaderType), RHIShader(nullptr)
{
}

Shader::Shader(const std::string& name, const std::string& shaderSourceCodePath, ERHIShaderFrequency shaderType)
    : Name(name), ShaderType(shaderType), RHIShader(nullptr)
{


}


// Destructor
Shader::~Shader()
{
    // Cleanup if necessary
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

} // namespace RenderCore