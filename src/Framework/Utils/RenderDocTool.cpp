#include "RenderDocTool.h"
#include <iostream>
#include <Windows.h>
void RenderDocTool::CaptureBegin()
{
    if (numCaptures == 0)
    {
        return;
    }

    // 启动 RenderDoc 捕获
    rdoc_api->StartFrameCapture(nullptr, nullptr);
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
    rdoc_api->EndFrameCapture(nullptr, nullptr);
  
}

void RenderDocTool::SetCaptureOutPath(std::string outPath)
{
    captureOutPath = outPath;
    rdoc_api->SetCaptureFilePathTemplate(captureOutPath.c_str());
    //rdoc_api->SetLogFilePathTemplate(captureOutPath.c_str());
}

void RenderDocTool::SetNumCaptures(uint32_t numCapture)
{
    numCaptures = numCapture;
}

void RenderDocTool::WriteCaptureOut()
{
    uint32_t numCapture = rdoc_api->GetNumCaptures();
    rdoc_api->TriggerCapture();
    //if (numCapture)
    //{
    //    rdoc_api->GetCapture()
    //}

}

bool RenderDocTool::IsCapturing()
{
    auto res = rdoc_api->IsFrameCapturing();
    return res;
}

void RenderDocTool::Init()
{
    // 在需要捕获的地方调用这个函数
    //RENDERDOC_API_1_1_2* rdoc_api = nullptr;
   
    HMODULE renderdoc_module = LoadLibraryA("D:/RenderDoc/renderdoc.dll"); // 替换为RenderDoc的实际DLL名称
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

    // 在这里检查 rdoc_api 是否成功获取
    // ...

    // 初始化RenderDoc
    
}
