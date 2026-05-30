#pragma once
#include <vector>

namespace RenderCore {

    class RENDERCORE_API ShaderCompiledBinaryResultPacker {
    public:
        virtual void Depack(const std::vector<char>& packedResult) = 0;
    protected:
        virtual bool Pack(void* packSource, std::vector<char>& packedResultOut) { return true; };
    };

    class RENDERCORE_API SPIRVCompiledBinaryResultPacker : public ShaderCompiledBinaryResultPacker {
    public:
        enum class ESPIRVShaderResourceType : uint8_t
        {
            Sampler = 0,
            SampledImage = 1,
            StorageImage = 2,
            UniformBuffer = 3,
            StorageBuffer = 4,
            // 补充类型
            UniformTexelBuffer = 5,    // tN: Buffer<T>
            StorageTexelBuffer = 6,    // uN: RWBuffer<T>

            // 特殊提交方式
            PushConstant = 7,          // 这种资源没有 Binding，只有 Offset

            // 现代 API 特性
            AccelerationStructure = 8, // 光追加速结构
            Num = 9

        };

        struct DescriptorBindingInfo
        {
            uint16_t Set = 0;
            uint16_t Binding = 0;
            uint16_t Count = 1;

            ESPIRVShaderResourceType Type;
            char Name[32] = {};
        };

        struct UniformBufferBindingInfo 
        {
			uint16_t Set = 0;
			uint16_t Binding = 0;
			uint16_t Size = 0;
			char Name[32] = {};
        };

        struct PushConstantInfo
        {
            uint16_t Size = 0;
        };

        struct Header
        {
            RHI::ERHIShaderFrequency Frequency = RHI::ERHIShaderFrequency::Unknown;

            char EntryPoint[32] = "main";

            uint64_t ShaderHash = 0;

            std::vector<DescriptorBindingInfo> DescriptorBindings;
            bool HasPushConstant = false;
            PushConstantInfo PushConstant;
            std::vector<UniformBufferBindingInfo> UniformBufferBindings;
        };

        struct PackedCode {
            std::vector<uint32_t> SpirvCode;
            Header header;

        } DepackedData;

        void Depack(const std::vector<char>& packedResult) override;
    protected:
        bool Pack(void* packSource, std::vector<char>& packedResultOut) override;
        friend class ShaderCompiler;
    };


}