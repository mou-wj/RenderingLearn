#include "PathInfo.h"
#include "Math.hpp"
#include "Module.h"
#include "BoxSphereBounds.h"
namespace Core {
	std::string GetProjectDir()
	{
		return PROJECT_DIR;
	}
	CORE_API std::string GetExecutableDir() {
        return EXECUTABLE_DIR;
	}

}
