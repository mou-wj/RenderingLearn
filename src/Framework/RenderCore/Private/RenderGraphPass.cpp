#include "RenderGraphPass.h"
#include <cassert>

namespace RenderCore {

    RenderGraphPass::RenderGraphPass(const std::string& name, EPassFlag passFlag, const RenderGraphParameterStruct& parameter)
        : Name(name),PassFlag(passFlag), ParameterStruct(parameter)
    {

    }


} // namespace RenderCore