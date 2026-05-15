#pragma once

#include "GlobalShader.h"
#include "ShaderParameter.h"
#include "RHICommandList.h"
#include "ShaderCore.h"
namespace RenderCore {



    // ���� Blit �����ṹ
    BEGIN_SHADER_PARAMETER_STRUCT(BlitTextureParameters)
        SHADER_PARAMETER(Core::Float2, SrcSize)
        SHADER_PARAMETER(Core::Float2, SrcInvSize)
        SHADER_PARAMETER(Core::Float2, DstSize)
        SHADER_PARAMETER(Core::Float2, DstInvSize)
        SHADER_PARAMETER(float, SrcMipLevel)
        SHADER_PARAMETER_TEXTURE(RenderGraphTexture, SrcTexture)
        SHADER_PARAMETER_TEXTURE_UAV(RenderGraphTextureUAV, DstTexture)
		SHADER_PARAMETER_SAMPLER(SrcSampler)
    END_SHADER_PARAMETER_STRUCT(BlitTextureParameters)

    class BlitTextureCS : public GlobalShader
    {
        
    public:
        static constexpr char Macro_ColorMode[] = "COLOR_CONVERSION_MODE";

        // 2. 定义变体维度（例如 3 种颜色转换模式）
        using ColorConversionDim = FPermutationDimensionEnum<Macro_ColorMode, 3>;

        // 3. 定义变体域（Domain），可以包含多个维度
        using PermutationDomain = ShaderPermutationDomain<ColorConversionDim>;

        

        // ����Ϊ Global Shader ����
        // ��һ��������� IMPLEMENT_SHADER_TYPE_FLAG ����ע���߼�
        DECLARE_GLOBAL_SHADER_TYPE(BlitTextureCS);

        // Ĭ�������ȫ�� Shader �������б���
        static bool ShouldCompilePermutation(const ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        // �޸ı��뻷���������Ҫ����ĳЩ�꣬����֧�ֲ�ͬ�Ĳ�����ʽ��
        static void ModifyShaderCompilerEnvironment(const ShaderPermutationParameters& Parameters,ShaderCompilerEnvironment& OutEnvironment)
        {
            PermutationDomain Domain;
            Domain.SetFromId(Parameters.PermutationId);
            Domain.ModifyCompilationEnvironment(OutEnvironment);
        }

        // ��ȡ����Ԫ���ݣ����ڷ���
        static const ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return &BlitTextureParameters::GetMetaData();
        }
    };



} // namespace RenderCore