#include "RenderGraphPass.h"
#include <cassert>

namespace RenderCore {

    RenderGraphPass::RenderGraphPass(const std::string& name, EPassFlag passFlag, const RenderGraphParameterStruct& parameter)
        : Name(name),PassFlag(passFlag), ParameterStruct(parameter)
    {

    }


    void BarrierBatchBegin::Process()
    {
        for (size_t i = 0; i < transitions.size(); ++i)
        {
            auto& t = transitions[i];
            auto* res = resources[i];

            if (!res)
                continue;

            switch (t.Type)
            {
            case RHI::RHITransitionInfo::EType::Texture:
            {
                auto* tex = static_cast<RenderGraphTexture*>(res);
                t.Texture = tex->GetRHITexture();

                // assert(t.Texture && "Texture not allocated before barrier resolve");

                break;
            }
            case RHI::RHITransitionInfo::EType::Buffer:
            {
                auto* buf = static_cast<RenderGraphBuffer*>(res);
                t.Buffer = buf->GetRHIBuffer();

                // assert(t.Buffer && "Buffer not allocated before barrier resolve");

                break;
            }
            default:
                break;
            }
        }
    }

    void BarrierBatchEnd::Process()
    {
        for (size_t i = 0; i < transitions.size(); ++i)
        {
            auto& t = transitions[i];
            auto* res = resources[i];

            if (!res)
                continue;

            switch (t.Type)
            {
            case RHI::RHITransitionInfo::EType::Texture:
            {
                auto* tex = static_cast<RenderGraphTexture*>(res);
                t.Texture = tex->GetRHITexture();
                break;
            }
            case RHI::RHITransitionInfo::EType::Buffer:
            {
                auto* buf = static_cast<RenderGraphBuffer*>(res);
                t.Buffer = buf->GetRHIBuffer();
                break;
            }
            default:
                break;
            }
        }
    }

} // namespace RenderCore