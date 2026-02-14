#include "RHICommandContex.h"
#include "RHICommandList.h"
#include "RHIDefine.h"

namespace RHI
{
	RHICommandContex::RHICommandContex() : CommandList(this)
	{
		CommandList.SetImmediate(true);
	}
}
