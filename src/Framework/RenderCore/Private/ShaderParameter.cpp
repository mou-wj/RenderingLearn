#include "ShaderParameter.h"
#include "Shader.h"
#include "RenderGraphResource.h"
namespace RenderCore {


    class RENDERCORE_API ShaderParameterReader
    {
    public:
        // ����ʱֱ�Ӵ��� C++ �ṹ��ʵ����ָ��
        ShaderParameterReader(const void* InData, uint32_t InSize)
            : Data(reinterpret_cast<const uint8_t*>(InData))
            , DataSize(InSize)
        {
        }

        /**
         * ���ģ�����һ��ӳ��õ� Binding ��Ϣ��ֱ�Ӵ��ڴ�����г�����
         */
        template<typename T>
        const T& Read(const ShaderParameterBindingInfo::ShaderUniformBinding& Binding) const
        {
            // ��ȫ��飺ȷ����ȡ��ΧûԽ��
            // ����� Binding.Offset �Ǹó�Ա�� C++ �ṹ���е�ƫ��
            assert(Binding.Offset + sizeof(T) <= DataSize);

            return *reinterpret_cast<const T*>(Data + Binding.Offset);
        }

        // ��ȡԭʼָ�룬���� memcpy �������
        const void* GetRawPointer(uint32_t Offset) const
        {
            assert(Offset <= DataSize);
            return Data + Offset;
        }

    private:
        const uint8_t* Data;
        const uint32_t DataSize;
    };


    void SetShaderParameters(
        RHI::RHICommandListBase& cmdList,
        const Shader* shader,
        const ShaderParametersMetadata* ParametersMetaData,
        void* ParametersData) {
        if (!shader || !ParametersData) return;

        if (!shader || !ParametersData) return;

        // 1. ��ʼ�� Reader (�������ģʽ)
        // ע�⣺����� DataSize ʹ�� Metadata ����Ĵ�С
        ShaderParameterReader Reader(ParametersData, ParametersMetaData->GetSize());

        // 2. ׼�� RHI ����������
        RHI::RHIBatchedShaderParameters BatchedParams;

        // 3. ���� Shader �� Bindings �б� (�� Shader ������ȡ)
        // ������֮ǰ���Ĳ�ͬ�����ǲ��ٵ��� Metadata�����ǵ��� Shader ������������
        const ShaderParameterBindingInfo& BindingInfo = shader->GetParameterBindings();

        // --- ������ֵ�� (Uniforms) ---
        size_t uniformDataOffset = 0;
        for (const auto& Pair : BindingInfo.UniformBindings)
        {
            const std::string& Name = Pair.first;
            const auto& Binding = Pair.second;

            // 组装Uniform参数描述
            RHI::RHIShaderUniformParameter param;
            param.BufferIndex = Binding.BufferIndex; // 若有多UBO可扩展
            param.BaseIndex = Binding.BaseIndex; // 这里用Offset或BindSlot视RHI实现
            param.Offset = uniformDataOffset;
            param.Size = Binding.Size;
            BatchedParams.UniformParameters.push_back(param);

            // 拷贝数据到Data
            const uint8_t* src = reinterpret_cast<const uint8_t*>(Reader.GetRawPointer(Binding.Offset));
            BatchedParams.Data.insert(BatchedParams.Data.end(), src, src + Binding.Size);
            uniformDataOffset += Binding.Size;
        }

        // --- 资源参数组装 ---
        for (const auto& Pair : BindingInfo.ResourceBindings)
        {
            const std::string& Name = Pair.first;
            const auto& Binding = Pair.second;
            const void* ResourcePtrLocation = Reader.GetRawPointer(Binding.Offset);
            RHI::RHIResource* Resource = nullptr;

            RHI::RHIShaderResourceParameter param;
            param.Index = Binding.BindSlot;


            // 根据BaseType类型组装参数
            switch (Binding.BaseType)
            {
            case EShaderParameterBaseType::RDGTexture:
                param.Type = RHI::RHIShaderResourceParameter::EType::Texture;
                Resource = (*(RenderGraphResource**)ResourcePtrLocation)->GetRHIResource();
                break;
            case EShaderParameterBaseType::RDGTexture_UAV:
            case EShaderParameterBaseType::RDGBuffer_UAV:
                param.Type = RHI::RHIShaderResourceParameter::EType::UAV;
                Resource = (*(RenderGraphResource**)ResourcePtrLocation)->GetRHIResource();
                break;
            case EShaderParameterBaseType::RDGBuffer:
            case EShaderParameterBaseType::RDGBuffer_SRV:
                param.Type = RHI::RHIShaderResourceParameter::EType::SRV;
                Resource = (*(RenderGraphResource**)ResourcePtrLocation)->GetRHIResource();
                break;
            case EShaderParameterBaseType::RHISampler:
                param.Type = RHI::RHIShaderResourceParameter::EType::Sampler;
                Resource = *(RHIResource**)ResourcePtrLocation;
                break;
            case EShaderParameterBaseType::StructNested:
                param.Type = RHI::RHIShaderResourceParameter::EType::UniformBuffer;
                break;
            default:
                assert(false);
                break;
            }
            param.Resource = Resource;
            BatchedParams.ResourceParameters.push_back(param);
        }

        // 4. �ύ��ָ����
        if (shader->GetShaderFrequency() == ERHIShaderFrequency::Compute)
        {
            auto& ComputeCmdList = static_cast<RHI::RHIComputeCommandList&>(cmdList);
            ComputeCmdList.SetBatchedShaderParameters(
                static_cast<RHI::RHIComputeShader*>(shader->GetRHIShader()),
                BatchedParams
            );
        }
        else
        {
            auto& GraphicCmdList = static_cast<RHI::RHIGraphicCommandList&>(cmdList);
            GraphicCmdList.SetBatchedShaderParameters(
                static_cast<RHI::RHIGraphicShader*>(shader->GetRHIShader()),
                BatchedParams
            );
        }

    }


}
