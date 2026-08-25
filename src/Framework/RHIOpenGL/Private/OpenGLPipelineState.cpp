#include "OpenGLPipelineState.h"
#include "OpenGLResource.h"
#include "glad/gl.h"
#include <cstdio>

namespace RHIOpenGL
{
    static bool LinkOpenGLProgram(GLuint programHandle)
    {
        if (programHandle == 0)
        {
            return false;
        }

        glLinkProgram(programHandle);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE)
        {
            GLint infoLogLength = 0;
            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::vector<char> infoLog(static_cast<size_t>(infoLogLength > 0 ? infoLogLength : 1));
            glGetProgramInfoLog(programHandle, static_cast<GLint>(infoLog.size()), nullptr, infoLog.data());
            std::fprintf(stderr, "[OpenGLRHI] Failed to link program: %s\n", infoLog.data());
            return false;
        }

        return true;
    }

    OpenGLGraphicsPipelineState::~OpenGLGraphicsPipelineState()
    {
        if (ProgramHandle != 0)
        {
            glDeleteProgram(ProgramHandle);
            ProgramHandle = 0;
        }
    }

    OpenGLGraphicsPipelineState::OpenGLGraphicsPipelineState(const RHI::RHIGraphicsPipelineStateDesc& desc)
        : RHI::RHIGraphicsPipelineState(desc)
    {
        ProgramHandle = glCreateProgram();

        if (desc.shaderStages.vertexShader)
        {
            auto* vertexShader = dynamic_cast<OpenGLVertexShader*>(desc.shaderStages.vertexShader);
            if (vertexShader && vertexShader->GetHandle() != 0)
            {
                glAttachShader(ProgramHandle, vertexShader->GetHandle());
            }
        }

        if (desc.shaderStages.fragmentShader)
        {
            auto* fragmentShader = dynamic_cast<OpenGLFragmentShader*>(desc.shaderStages.fragmentShader);
            if (fragmentShader && fragmentShader->GetHandle() != 0)
            {
                glAttachShader(ProgramHandle, fragmentShader->GetHandle());
            }
        }

        LinkOpenGLProgram(ProgramHandle);
    }

    OpenGLComputePipelineState::~OpenGLComputePipelineState()
    {
        if (ProgramHandle != 0)
        {
            glDeleteProgram(ProgramHandle);
            ProgramHandle = 0;
        }
    }

    OpenGLComputePipelineState::OpenGLComputePipelineState(const RHI::RHIComputePipelineStateDesc& desc)
        : RHI::RHIComputePipelineState(desc)
    {
        ProgramHandle = glCreateProgram();

        if (desc.computeShader)
        {
            auto* computeShader = dynamic_cast<OpenGLComputeShader*>(desc.computeShader);
            if (computeShader && computeShader->GetHandle() != 0)
            {
                glAttachShader(ProgramHandle, computeShader->GetHandle());
            }
        }

        LinkOpenGLProgram(ProgramHandle);
    }
}
