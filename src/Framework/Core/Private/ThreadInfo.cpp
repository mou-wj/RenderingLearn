#include "ThreadInfo.h"
#include <thread>
namespace Core {

	std::thread::id GRenderThreadID;
	void InitRenderThreadId()
	{
		GRenderThreadID = std::this_thread::get_id();
	}
	bool IsInRenderThread()
	{
		return GRenderThreadID == std::this_thread::get_id();
	}

}

