#pragma once
#include <string>
#include "../Common/Template.hpp"
struct RENDERDOC_API_1_6_0;


//renderdoc捕获数据可能会使用device创建一些资源，这可能导致销毁device时报错没有资源被销毁,且在循环中捕获帧使用不当可能还会引起同步问题，导致提交执行失败
//renderdoc可能还会导致某些设备拓展无线正常显示，具体原因目前不明
class RenderDocTool:public SingleInstance<RenderDocTool> {
	friend class SingleInstance<RenderDocTool>;
public:
	void CaptureBegin();
	void CaptureEnd();
	void SetCaptureOutPath(std::string outPath);
	void SetNumCaptures(uint32_t numCapture);
	void WriteCaptureOut();
	void SetDevice(void* device);
	bool IsCapturing();
private:
	void Init();
	std::string captureOutPath;
	void* devicePointer = nullptr;
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
#define CaotureSetDevice(device) RenderDocTool::GetInstance()->SetDevice(device);