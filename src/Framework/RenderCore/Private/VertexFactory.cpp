#include "VertexFactory.h"

namespace RenderCore {
	RHIVertexDescStateSP VertexFactory::GetRHIVertexDescState() const
	{
		return RHIVertexDescState;
	}
} // namespace RenderCore