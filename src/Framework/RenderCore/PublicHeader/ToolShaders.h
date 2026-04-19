#pragma once

#include "GlobalShader.h"
#include "ShaderParameter.h"
#include "RHICommandList.h"
namespace RenderCore {

    // 定义 Blit 参数结构
    BEGIN_SHADER_PARAMETER_STRUCT(FBlitCopyTextureParameters)
    END_SHADER_PARAMETER_STRUCT(FBlitCopyTextureParameters)

    class FBlitCopyTextureCS : public GlobalShader
    {
        
    public:
        // 声明为 Global Shader 类型
        // 这一步会由你的 IMPLEMENT_SHADER_TYPE_FLAG 处理注册逻辑
        DECLARE_GLOBAL_SHADER_TYPE(FBlitCopyTextureCS);

        // 默认情况下全局 Shader 编译所有变体
        static bool ShouldCompilePermutation(const ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        // 修改编译环境（如果需要开启某些宏，比如支持不同的采样方式）
        static void ModifyShaderCompilerEnvironment(ShaderCompilerEnvironment& OutEnvironment)
        {
            GlobalShader::ModifyShaderCompilerEnvironment(OutEnvironment);
            // OutEnvironment.SetDefine("SUPPORT_SRGB", 1);
        }

        // 获取参数元数据，用于反射
        static const ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return &FBlitCopyTextureParameters::GetMetaData();
        }
    };

    // --- 注册 Shader ---
    // 使用你的宏，标记为 Global 分组
    IMPLEMENT_SHADER_TYPE_FLAG(
        FBlitCopyTextureCS,
        "BlitCopyTextureCS",                     // Shader 友好名称
        "/Engine/Shaders/BlitCopyTexture.usf",   // 虚拟路径
        "MainCS",                                // 入口点
        RHI::ERHIShaderFrequency::Compute,       // 频率
        ShaderType::EShaderTypeFlag::Global      // 注册分组：Global
    );

} // namespace RenderCore