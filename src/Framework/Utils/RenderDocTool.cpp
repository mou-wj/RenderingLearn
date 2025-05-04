#include "RenderDocTool.h"
#include <iostream>
#include <Windows.h>

#ifdef ENABLE_RENDERDOC_CAPTURE
#include <renderdoc_app.h>
#endif

void RenderDocTool::CaptureBegin()
{
    if (numCaptures == 0)
    {
        return;
    }

    // 启动 RenderDoc 捕获
#ifdef ENABLE_RENDERDOC_CAPTURE
    rdoc_api->StartFrameCapture(devicePointer, nullptr);
#endif

    numCaptures--;
}

void RenderDocTool::CaptureEnd()
{
    if (numCaptures == 0)
    {
        return;
    }
    // 使用之前获取到的 API 指针
    // ...

    // 结束 RenderDoc 捕获
#ifdef ENABLE_RENDERDOC_CAPTURE
    rdoc_api->EndFrameCapture(devicePointer, nullptr);
#endif

  
}

void RenderDocTool::SetCaptureOutPath(std::string outPath)
{
    captureOutPath = outPath;
#ifdef ENABLE_RENDERDOC_CAPTURE
    rdoc_api->SetCaptureFilePathTemplate(captureOutPath.c_str());
#endif

}

void RenderDocTool::SetNumCaptures(uint32_t numCapture)
{
    numCaptures = numCapture;
}

void RenderDocTool::WriteCaptureOut()
{

#ifdef ENABLE_RENDERDOC_CAPTURE
    uint32_t numCapture = rdoc_api->GetNumCaptures();

    rdoc_api->TriggerCapture();
#endif

    //if (numCapture)
    //{
    //    rdoc_api->GetCapture()
    //}

}

void RenderDocTool::SetDevice(void* device)
{
    devicePointer = device;
}

bool RenderDocTool::IsCapturing()
{
    bool res = false;
#ifdef ENABLE_RENDERDOC_CAPTURE
    res = rdoc_api->IsFrameCapturing();
#endif

    return res;
}

void RenderDocTool::Init()
{
    // 在需要捕获的地方调用这个函数
    //RENDERDOC_API_1_1_2* rdoc_api = nullptr;
#ifdef ENABLE_RENDERDOC_CAPTURE
    std::string renderdocDll = std::string(Renderdoc_INSTALL_DIR) + "/renderdoc.dll";
    HMODULE renderdoc_module = LoadLibraryA(renderdocDll.c_str()); // 替换为RenderDoc的实际DLL名称
    if (renderdoc_module != nullptr)
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(renderdoc_module, "RENDERDOC_GetAPI"));

        if (RENDERDOC_GetAPI != nullptr)
        {
            RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, reinterpret_cast<void**>(&rdoc_api));
            int major = 0, minor = 0, match = 0;
            rdoc_api->GetAPIVersion(&major, &minor, &match);
            std::cout << major << " " << minor << " " << match << std::endl;
        }

    }
#endif

}
