#include "MaterialShaderParameter.h"
#include "Material.h"
#include "RenderGraphBuilder.h"
#include "RenderResource.h"
using namespace RHI;
using namespace RenderCore;
namespace Engine {

    void BuildShaderParameters(
            const MaterialRenderProxy*
            Proxy,
            RenderCore::RenderGraphBuilder& Builder,
            MaterialShaderParameters&
            Out)
    {
        // ====================
        // default init
        // ====================

        Out.UniformData
            .BaseColor =
        { 1,1,1,1 };

        Out.UniformData
            .Metallic = 0.0f;

        Out.UniformData
            .Roughness = 0.5f;

        Out.UniformData
            .Specular = 0.5f;
        Out.UniformData.AmbientOcclusion = 1;
        Out.TextureData
            .TextureMask = 0;

        RenderCore::RenderGraphTextureDesc emptyDesc;
        emptyDesc.Format = ERHIFormat::R8G8B8A8_UNorm;
        emptyDesc.Width = 1;
        emptyDesc.Height = 1;
        emptyDesc.Usage = ERHITextureCreateFlag::ShaderResource;

		auto emptyTexture = Builder.CreateTexture(
			"EmptyTexture",
            emptyDesc);

        //默认初始化纹理为空纹理
        Out.TextureData.BaseColorTexture = emptyTexture;
        Out.TextureData.OpacityTexture = emptyTexture;
        Out.TextureData.NormalTexture = emptyTexture;
        Out.TextureData.ORMTexture = emptyTexture;
        Out.TextureData.EmissiveTexture = emptyTexture;
        Out.SamplerData.LinearWrapSampler = GlobalSampler.get();

        // ====================
        // scalar
        // ====================

        float Scalar;

        if (Proxy->GetValue<
            EMaterialParameterSemantic
            ::Metallic,
            FScalarType>(
                "Metallic",
                Scalar))
        {
            Out.UniformData
                .Metallic =
                Scalar;
        }

        if (Proxy->GetValue<
            EMaterialParameterSemantic
            ::Roughness,
            FScalarType>(
                "Roughness",
                Scalar))
        {
            Out.UniformData
                .Roughness =
                Scalar;
        }

        // ====================
        // vector
        // ====================

        std::array<float, 4>
            Color;

        if (Proxy->GetValue<
            EMaterialParameterSemantic
            ::BaseColor,
            FVectorType>(
                "BaseColor",
                Color))
        {
            Out.UniformData
                .BaseColor =
            {
                Color[0],
                Color[1],
                Color[2],
                Color[3]
            };
        }

        // ====================
        // textures
        // ====================

        RenderCore
            ::RenderTexture*
            Texture;

        if (Proxy->GetValue<
            EMaterialParameterSemantic
            ::BaseColor,
            FTextureType>(
                "BaseColor",
                Texture))
        {
            auto rdgBaseColorTexture = Builder.RegisterExternalTexture("BaseColorTexture",
                Texture);
            Out.TextureData
                .BaseColorTexture =
                rdgBaseColorTexture;

            Out.TextureData
                .TextureMask
                |= 1 << 0;
        }

        if (Proxy->GetValue<
            EMaterialParameterSemantic
            ::Normal,
            FTextureType>(
                "Normal",
                Texture))
        {
            auto rdgNormalTexture = Builder.RegisterExternalTexture("NormalTexture",
                Texture);
            Out.TextureData
                .NormalTexture =
                rdgNormalTexture;

            Out.TextureData
                .TextureMask
                |= 1 << 1;
        }
    }


}