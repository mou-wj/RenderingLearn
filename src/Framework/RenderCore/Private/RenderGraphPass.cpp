#include "RenderGraphPass.h"
#include <cassert>

namespace RenderCore {

    RenderGraphPass::RenderGraphPass(const std::string& name, const RenderGraphPassInfo& info)
        : Name(name), PassInfo(info)
    {
        // If no shader parameter block provided, nothing to extract
        if (PassInfo.ShaderParmeters.Paramters == nullptr)
            return;

        // Create a reader for the shader parameter block
        ShaderParameterReader reader(PassInfo.ShaderParmeters.Paramters, PassInfo.ShaderParmeters.ContentMetaData);
        ShaderParameterInstance inst;

        // Extract all non-struct parameters and store into pass caches depending on type
        while (reader.ReadNext(inst)) {
            switch (inst.Type) {
            case EShaderUniformBaseType::Texture: {
                // A direct texture SP stored in the param block
                auto texPtr = reinterpret_cast<RenderGraphTextureSP*>(inst.Ptr);
                if (texPtr && *texPtr) {
                    ReadTextureResourceCache = *texPtr;
                }
                break;
            }
            case EShaderUniformBaseType::Texture_SRV: {
                auto srvPtr = reinterpret_cast<RenderGraphTextureSRVSP*>(inst.Ptr);
                if (srvPtr && *srvPtr) {
                    ReadOnlyTextureCache = *srvPtr;
                    // also cache underlying texture resource for convenience
                    if ((*srvPtr)->GetDesc().Texture) {
                        ReadTextureResourceCache = (*srvPtr)->GetDesc().Texture;
                    }
                }
                break;
            }
            case EShaderUniformBaseType::Texture_UAV: {
                auto uavPtr = reinterpret_cast<RenderGraphTextureUAVSP*>(inst.Ptr);
                if (uavPtr && *uavPtr) {
                    ReadWriteTextureCache = *uavPtr;
                    // also cache underlying texture resource
                    if ((*uavPtr)->GetDesc().Texture) {
                        ReadTextureResourceCache = (*uavPtr)->GetDesc().Texture;
                    }
                }
                break;
            }
            case EShaderUniformBaseType::Buffer: {
                auto bufPtr = reinterpret_cast<RenderGraphBufferSP*>(inst.Ptr);
                if (bufPtr && *bufPtr) {
                    ReadWriteBufferResourceCache = *bufPtr;
                }
                break;
            }
            case EShaderUniformBaseType::Buffer_SRV: {
                auto bufSrvPtr = reinterpret_cast<RenderGraphBufferSRVSP*>(inst.Ptr);
                if (bufSrvPtr && *bufSrvPtr) {
                    ReadOnlyBufferCache = *bufSrvPtr;
                    // also cache underlying buffer resource
                    if ((*bufSrvPtr)->GetDesc().Buffer) {
                        ReadWriteBufferResourceCache = (*bufSrvPtr)->GetDesc().Buffer;
                    }
                }
                break;
            }
            case EShaderUniformBaseType::Buffer_UAV: {
                auto bufUavPtr = reinterpret_cast<RenderGraphBufferUAVSP*>(inst.Ptr);
                if (bufUavPtr && *bufUavPtr) {
                    ReadWriteBufferCache = *bufUavPtr;
                    // also cache underlying buffer resource
                    if ((*bufUavPtr)->GetDesc().Buffer) {
                        ReadWriteBufferResourceCache = (*bufUavPtr)->GetDesc().Buffer;
                    }
                }
                break;
            }
            case EShaderUniformBaseType::ColorBindings: {
                // Render target binding parameter: may contain multiple color targets + depth
                auto rtPtr = reinterpret_cast<RenderTargetBindingParameter*>(inst.Ptr);
                if (rtPtr) {
                    // populate ReadTextureResourceCache with first non-null (or overwrite with last).
                    for (const auto& ct : rtPtr->ColorTargets) {
                        if (ct.Texture) {
                            ReadTextureResourceCache = ct.Texture;
                        }
                    }
                    if (rtPtr->DepthStencilTarget) {
                        ReadTextureResourceCache = rtPtr->DepthStencilTarget;
                    }
                }
                break;
            }
            default:
                // Non-resource parameter: ignore here
                break;
            } // switch
        } // while
    }

    void BarrierBatchBegin::Execute(RHICommandList& commandList)
    {
    }

    void BarrierBatchEnd::Execute(RHICommandList& commandList)
    {
    }

} // namespace RenderCore