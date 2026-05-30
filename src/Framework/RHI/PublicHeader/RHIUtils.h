#pragma once
#include "RHIDefine.h"

namespace RHI {

	RHI_API size_t GetFormatSize(ERHIFormat format);

	RHI_API EVerdorId GetVendorIdFromUint32(uint32_t vendorId);
	
	RHI_API EVerdorId GetPreferredVendorId();


}