#pragma once
#include <string>
#include "../Common/Template.hpp"
#include <renderdoc_app.h>

class RenderDocTool:public SingleInstance<RenderDocTool> {
	friend class SingleInstance<RenderDocTool>;
public:
	void CaptureBegin();
	void CaptureEnd();
	void SetCaptureOutPath(std::string outPath);
	void SetNumCaptures(uint32_t numCapture);
	void WriteCaptureOut();
	bool IsCapturing();
private:
	void Init();
	std::string captureOutPath;
	RENDERDOC_API_1_6_0* rdoc_api = nullptr;
	uint32_t numCaptures = 1;
	RenderDocTool() {
		Init();
	}
};
#define CaptureOutPathSetMacro(x) RenderDocTool::GetInstance()->SetCaptureOutPath(x);
#define CaptureBeginMacro RenderDocTool::GetInstance()->CaptureBegin();
#define CaptureEndMacro RenderDocTool::GetInstance()->CaptureEnd();
#define IsRenderDocCapturing RenderDocTool::GetInstance()->IsCapturing()
#define CaptureNum(num) RenderDocTool::GetInstance()->SetNumCaptures(num);