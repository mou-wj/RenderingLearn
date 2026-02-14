#include "VertexFactory.h"

namespace RenderCore {
	RHI::RHIVertexDescStateSP VertexFactory::GetRHIVertexDescState() const
	{
		return RHIVertexDescState;
	}
} // namespace RenderCore