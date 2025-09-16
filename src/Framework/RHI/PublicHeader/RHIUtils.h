#pragma once
#include "RHIDefine.h"

namespace RHI {

	size_t GetFormatSize(ERHIFormat format);

	EVerdorId GetVendorIdFromUint32(uint32_t vendorId);
	
	EVerdorId GetPreferredVendorId();


}