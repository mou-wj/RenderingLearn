#pragma once

#include "GlobalShader.h"
#include "ShaderParameter.h"
#include "RHICommandList.h"
namespace RenderCore {


    // ���� Blit �����ṹ
    BEGIN_SHADER_PARAMETER_STRUCT(BlitTextureParameters)
        SHADER_PARAMETER(Core::Float2, SrcSize)
        SHADER_PARAMETER(Core::Float2, SrcInvSize)
        SHADER_PARAMETER(Core::Float2, DstSize)
        SHADER_PARAMETER(Core::Float2, DstInvSize)
        SHADER_PARAMETER(float, SrcMipLevel)
    END_SHADER_PARAMETER_STRUCT(BlitTextureParameters)

    class BlitTextureCS : public GlobalShader
    {
        
    public:
        // ����Ϊ Global Shader ����
        // ��һ��������� IMPLEMENT_SHADER_TYPE_FLAG ����ע���߼�
        DECLARE_GLOBAL_SHADER_TYPE(BlitTextureCS);

        // Ĭ�������ȫ�� Shader �������б���
        static bool ShouldCompilePermutation(const ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        // �޸ı��뻷���������Ҫ����ĳЩ�꣬����֧�ֲ�ͬ�Ĳ�����ʽ��
        static void ModifyShaderCompilerEnvironment(ShaderCompilerEnvironment& OutEnvironment)
        {
            GlobalShader::ModifyShaderCompilerEnvironment(OutEnvironment);
            // OutEnvironment.SetDefine("SUPPORT_SRGB", 1);
        }

        // ��ȡ����Ԫ���ݣ����ڷ���
        static const ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return &BlitTextureParameters::GetMetaData();
        }
    };

    // --- ע�� Shader ---
    // ʹ����ĺ꣬���Ϊ Global ����
    IMPLEMENT_SHADER_TYPE_FLAG(
        BlitTextureCS,
        "litTextureCS",                     // Shader �Ѻ�����
        "/tools/BlitTextureCS.sf",   // ����·��
        "MainCS",                                // ��ڵ�
        RHI::ERHIShaderFrequency::Compute,       // Ƶ��
        ShaderType::EShaderTypeFlag::Global      // ע����飺Global
    );

} // namespace RenderCore