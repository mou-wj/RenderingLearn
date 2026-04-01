#include "RHICommandContex.h"
#include "RHICommandList.h"
#include "RHIDefine.h"

namespace RHI
{
	RHIComputeContext::RHIComputeContext() : CommandList(this)
	{
		CommandList.SetImmediate(true);
	}
}
