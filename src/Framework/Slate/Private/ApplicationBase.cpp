#include "ApplicationBase.h"
namespace Slate{
	ApplicationBase* CurApplication = nullptr;
	ApplicationBase* ApplicationBase::GetApplication() {
		return CurApplication;
	}
	void ApplicationBase::SetApplication(ApplicationBase* InApplication) {
		CurApplication = InApplication;
	}





}