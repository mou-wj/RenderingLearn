#include "OpenGLRHIApi.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace RHIOpenGL
{
    static GLenum GetOpenGLTextureFormat(RHI::ERHIFormat format)
    {
        switch (format)
        {
        case RHI::ERHIFormat::R8_UNorm:
        case RHI::ERHIFormat::R32_Float:
            return GL_RED;
        case RHI::ERHIFormat::R16G16_Float:
        case RHI::ERHIFormat::R32G32_Float:
            return GL_RG;
        case RHI::ERHIFormat::B8G8R8A8_UNorm:
            return GL_BGRA;
        default:
            return GL_RGBA;
        }
    }

    static GLenum GetOpenGLTextureType(RHI::ERHIFormat format)
    {
        switch (format)
        {
        case RHI::ERHIFormat::R8_UNorm:
        case RHI::ERHIFormat::R8G8B8A8_UNorm:
        case RHI::ERHIFormat::B8G8R8A8_UNorm:
            return GL_UNSIGNED_BYTE;
        case RHI::ERHIFormat::R16G16_Float:
        case RHI::ERHIFormat::R16G16B16A16_Float:
            return GL_HALF_FLOAT;
        default:
            return GL_FLOAT;
        }
    }

 #if defined(_WIN32)
    static GLADapiproc LoadOpenGLProcAddress(const char* name)
    {
        PROC proc = wglGetProcAddress(name);
        const auto procValue = reinterpret_cast<uintptr_t>(proc);
        if (proc == nullptr || procValue == 1 || procValue == 2 || procValue == 3 || procValue == static_cast<uintptr_t>(-1))
        {
            static HMODULE openGLModule = GetModuleHandleA("opengl32.dll");
            if (!openGLModule)
            {
                openGLModule = LoadLibraryA("opengl32.dll");
            }
            proc = openGLModule ? GetProcAddress(openGLModule, name) : nullptr;
        }

        return reinterpret_cast<GLADapiproc>(proc);
    }

    static LRESULT CALLBACK OpenGLBootstrapWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return DefWindowProcA(window, message, wParam, lParam);
    }

    static bool RegisterOpenGLBootstrapWindowClass(HINSTANCE instance)
    {
        WNDCLASSA windowClass{};
        windowClass.style = CS_OWNDC;
        windowClass.lpfnWndProc = OpenGLBootstrapWindowProc;
        windowClass.hInstance = instance;
        windowClass.lpszClassName = "RenderingLearnOpenGLBootstrap";

        if (RegisterClassA(&windowClass) != 0)
        {
            return true;
        }

        return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    static HGLRC CreateOpenGLBootstrapContext(HDC deviceContext)
    {
        PIXELFORMATDESCRIPTOR pixelFormatDescriptor{};
        pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pixelFormatDescriptor.nVersion = 1;
        pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
        pixelFormatDescriptor.cColorBits = 32;
        pixelFormatDescriptor.cDepthBits = 24;
        pixelFormatDescriptor.cStencilBits = 8;
        pixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;

        const int pixelFormat = ChoosePixelFormat(deviceContext, &pixelFormatDescriptor);
        if (pixelFormat == 0 || !SetPixelFormat(deviceContext, pixelFormat, &pixelFormatDescriptor))
        {
            return nullptr;
        }

        return wglCreateContext(deviceContext);
    }

    using WGLCreateContextAttribsProc = HGLRC(WINAPI*)(HDC, HGLRC, const int*);

    static HGLRC TryCreateOpenGL43Context(HDC deviceContext, HGLRC shareContext)
    {
        auto createContextAttribs = reinterpret_cast<WGLCreateContextAttribsProc>(
            wglGetProcAddress("wglCreateContextAttribsARB"));
        if (!createContextAttribs)
        {
            return nullptr;
        }

        const int attributes[] = {
            0x2091, 4, // WGL_CONTEXT_MAJOR_VERSION_ARB
            0x2092, 3, // WGL_CONTEXT_MINOR_VERSION_ARB
            0x9126, 0x00000001, // WGL_CONTEXT_PROFILE_MASK_ARB: core profile
            0
        };
        return createContextAttribs(deviceContext, shareContext, attributes);
    }
 #endif

    OpenGLRHIApi::~OpenGLRHIApi() {
        Shutdown();
    }

    bool OpenGLRHIApi::Init()
    {
        RHI::GShaderPlatform = RHI::ERHIShaderPlatform::OpenGL;
        PlatformInfo.DepthRange = RHI::EDepthRange::ZeroToOne;
        PlatformInfo.EnableRayTracing = false;

 #if !defined(_WIN32)
        std::cerr << "[OpenGLRHI] Bootstrap context is currently implemented for Win32 only" << std::endl;
        return false;
 #else
        const HINSTANCE instance = GetModuleHandleA(nullptr);
        if (!RegisterOpenGLBootstrapWindowClass(instance))
        {
            std::cerr << "[OpenGLRHI] Failed to register bootstrap window class" << std::endl;
            return false;
        }

        HWND bootstrapWindow = CreateWindowExA(
            0,
            "RenderingLearnOpenGLBootstrap",
            "RenderingLearn OpenGL Bootstrap",
            WS_POPUP,
            0,
            0,
            1,
            1,
            nullptr,
            nullptr,
            instance,
            nullptr);
        if (!bootstrapWindow)
        {
            std::cerr << "[OpenGLRHI] Failed to create bootstrap window" << std::endl;
            return false;
        }

        HDC deviceContext = GetDC(bootstrapWindow);
        if (!deviceContext)
        {
            DestroyWindow(bootstrapWindow);
            std::cerr << "[OpenGLRHI] Failed to get bootstrap device context" << std::endl;
            return false;
        }

        HGLRC legacyContext = CreateOpenGLBootstrapContext(deviceContext);
        if (!legacyContext || !wglMakeCurrent(deviceContext, legacyContext))
        {
            if (legacyContext)
            {
                wglDeleteContext(legacyContext);
            }
            ReleaseDC(bootstrapWindow, deviceContext);
            DestroyWindow(bootstrapWindow);
            std::cerr << "[OpenGLRHI] Failed to create bootstrap WGL context" << std::endl;
            return false;
        }

        const int bootstrapVersion = gladLoadGL(LoadOpenGLProcAddress);
        if (bootstrapVersion == 0)
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(legacyContext);
            ReleaseDC(bootstrapWindow, deviceContext);
            DestroyWindow(bootstrapWindow);
            std::cerr << "[OpenGLRHI] gladLoadGL failed for bootstrap WGL context" << std::endl;
            return false;
        }

        HGLRC modernContext = TryCreateOpenGL43Context(deviceContext, nullptr);
        if (modernContext)
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(legacyContext);
            if (!wglMakeCurrent(deviceContext, modernContext))
            {
                wglDeleteContext(modernContext);
                ReleaseDC(bootstrapWindow, deviceContext);
                DestroyWindow(bootstrapWindow);
                std::cerr << "[OpenGLRHI] Failed to activate OpenGL 4.3 context" << std::endl;
                return false;
            }
            legacyContext = modernContext;
        }

        const int version = gladLoadGL(LoadOpenGLProcAddress);
        if (version == 0)
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(legacyContext);
            ReleaseDC(bootstrapWindow, deviceContext);
            DestroyWindow(bootstrapWindow);
            std::cerr << "[OpenGLRHI] gladLoadGL failed" << std::endl;
            return false;
        }

        BootstrapWindow = bootstrapWindow;
        BootstrapDeviceContext = deviceContext;
        BootstrapGLContext = legacyContext;
        // 3. 计算偏移量 (对齐 Header 之后的地址)
        RHI::G_RHITransition_PrivateDataOffset = 1;

        // 4. 计算总分配大小
        RHI::G_RHITransition_TotalSize = 2;


        std::cout << "[OpenGLRHI] OpenGL environment initialized. GL version: "
                  << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version)
                  << std::endl;
        return true;
 #endif
    }

    void OpenGLRHIApi::Shutdown()
    {
        DestroyBootstrapContext();
    }

    void OpenGLRHIApi::DestroyBootstrapContext()
    {
 #if defined(_WIN32)
        auto window = static_cast<HWND>(BootstrapWindow);
        auto deviceContext = static_cast<HDC>(BootstrapDeviceContext);
        auto glContext = static_cast<HGLRC>(BootstrapGLContext);

        if (glContext)
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(glContext);
        }
        if (window && deviceContext)
        {
            ReleaseDC(window, deviceContext);
        }
        if (window)
        {
            DestroyWindow(window);
        }
 #endif
        BootstrapWindow = nullptr;
        BootstrapDeviceContext = nullptr;
        BootstrapGLContext = nullptr;
    }

    const RHI::RHIPlatformInfo& OpenGLRHIApi::GetPlatformInfo() const
    {
        return PlatformInfo;
    }

    RHI::RHITextureSP OpenGLRHIApi::CreateTexture(const RHI::RHITextureDesc& desc)
    {
        return std::make_shared<OpenGLTexture>(desc);
    }

    RHI::RHIBufferSP OpenGLRHIApi::CreateBuffer(const RHI::RHIBufferDesc& desc)
    {
        return std::make_shared<OpenGLBuffer>(desc);
    }

    void OpenGLRHIApi::UpdateTexture(RHI::RHICommandListBase& cmdList, RHI::RHITexture* texture, const void* data, const RHI::RHIUpdateTextureRegion& region)
    {
        (void)cmdList;
        auto* glTexture = dynamic_cast<OpenGLTexture*>(texture);
        if (!glTexture || !data || region.width == 0 || region.height == 0 || region.depth == 0)
        {
            return;
        }

        const auto& desc = glTexture->GetDesc();
        if (region.mipLevel >= desc.MipLevels ||
            region.xOffset + region.width > desc.Width ||
            region.yOffset + region.height > desc.Height ||
            region.zOffset != 0 || region.depth != 1)
        {
            return;
        }

        glBindTexture(GL_TEXTURE_2D, glTexture->GetHandle());
        glTexSubImage2D(GL_TEXTURE_2D,
            static_cast<GLint>(region.mipLevel),
            static_cast<GLint>(region.xOffset),
            static_cast<GLint>(region.yOffset),
            static_cast<GLsizei>(region.width),
            static_cast<GLsizei>(region.height),
            GetOpenGLTextureFormat(desc.Format),
            GetOpenGLTextureType(desc.Format),
            data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLRHIApi::UpdateBuffer(RHI::RHICommandListBase& cmdList, RHI::RHIBuffer* buffer, const void* data, const RHI::RHIUpdateBufferRegion& region)
    {
        (void)cmdList;
        auto* glBuffer = dynamic_cast<OpenGLBuffer*>(buffer);
        if (!glBuffer || !data || region.size == 0 ||
            static_cast<uint64_t>(region.offset) + region.size > glBuffer->GetDesc().Size)
        {
            return;
        }

        glBindBuffer(GL_ARRAY_BUFFER, glBuffer->GetHandle());
        glBufferSubData(GL_ARRAY_BUFFER,
            static_cast<GLintptr>(region.offset),
            static_cast<GLsizeiptr>(region.size),
            data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void* OpenGLRHIApi::MapReadTexture(RHI::RHICommandListBase& cmdList, RHI::RHITexture* texture, const RHI::RHIReadTextureInfo& info)
    {
        (void)cmdList;
        auto* glTexture = dynamic_cast<OpenGLTexture*>(texture);
        if (!glTexture)
        {
            return nullptr;
        }

        const auto& desc = glTexture->GetDesc();
        if (info.MipLevel >= desc.MipLevels || info.ArraySlice >= desc.ArraySize)
        {
            return nullptr;
        }

        const auto formatInfo = RHI::GFormatInfoMap.find(desc.Format);
        if (formatInfo == RHI::GFormatInfoMap.end() || formatInfo->second.BytesPerPixel == 0)
        {
            return nullptr;
        }

        const uint32_t mipWidth = std::max(1u, desc.Width >> info.MipLevel);
        const uint32_t mipHeight = std::max(1u, desc.Height >> info.MipLevel);
        const size_t byteCount = static_cast<size_t>(mipWidth) * mipHeight * formatInfo->second.BytesPerPixel;
        void* mappedData = std::malloc(byteCount == 0 ? 1 : byteCount);
        if (!mappedData)
        {
            return nullptr;
        }

        glBindTexture(GL_TEXTURE_2D, glTexture->GetHandle());
        glGetTexImage(GL_TEXTURE_2D, static_cast<GLint>(info.MipLevel),
            GetOpenGLTextureFormat(desc.Format), GetOpenGLTextureType(desc.Format), mappedData);
        glBindTexture(GL_TEXTURE_2D, 0);
        return mappedData;
    }

    void* OpenGLRHIApi::MapReadBuffer(RHI::RHICommandListBase& cmdList, RHI::RHIBuffer* buffer, const RHI::RHIReadBufferInfo& info)
    {
        (void)cmdList;
        auto* glBuffer = dynamic_cast<OpenGLBuffer*>(buffer);
        if (!glBuffer || info.offset > glBuffer->GetDesc().Size)
        {
            return nullptr;
        }

        const uint64_t availableSize = glBuffer->GetDesc().Size - info.offset;
        const uint64_t readSize = info.size == 0 ? availableSize : info.size;
        if (readSize == 0 || readSize > availableSize)
        {
            return nullptr;
        }

        void* mappedData = std::malloc(static_cast<size_t>(readSize));
        if (!mappedData)
        {
            return nullptr;
        }

        glBindBuffer(GL_ARRAY_BUFFER, glBuffer->GetHandle());
        glGetBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(info.offset),
            static_cast<GLsizeiptr>(readSize), mappedData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return mappedData;
    }

    void OpenGLRHIApi::Unmap(void* mappedData)
    {
        std::free(mappedData);
    }

    RHI::RHIShaderResourceViewSP OpenGLRHIApi::CreateTextureShaderResourceView(RHI::RHITexture* Texture, const RHI::RHITexSRVCreateInfo& Desc)
    {
        return std::make_shared<OpenGLShaderResourceView>(Texture);
    }

    RHI::RHIUnorderedAccessViewSP OpenGLRHIApi::CreateTextureUnorderedAccessView(RHI::RHITexture* Texture, const RHI::RHITexUAVCreateInfo& Desc)
    {
        return std::make_shared<OpenGLUnorderedAccessView>(Texture);
    }

    RHI::RHIShaderResourceViewSP OpenGLRHIApi::CreateBufferShaderResourceView(RHI::RHIBuffer* Buffer, const RHI::RHIBufferSRVCreateInfo& Desc)
    {
        return std::make_shared<OpenGLShaderResourceView>(Buffer);
    }

    RHI::RHIUnorderedAccessViewSP OpenGLRHIApi::CreateBufferUnorderedAccessView(RHI::RHIBuffer* Buffer, const RHI::RHIBufferUAVCreateInfo& Desc)
    {
        return std::make_shared<OpenGLUnorderedAccessView>(Buffer);
    }

    RHI::RHIRayTracingGeometrySP OpenGLRHIApi::CreateRayTracingGeometry(const RHI::RHIRayTracingGeometryDesc& desc)
    {
        return nullptr;
    }

    RHI::RHIRayTracingInstanceSP OpenGLRHIApi::CreateRayTracingInstance(const RHI::RHIRayTracingInstancesDesc& desc)
    {
        return nullptr;
    }

    RHI::RHIStagingBufferSP OpenGLRHIApi::CreateStagingBuffer(uint32_t size)
    {
        return std::make_shared<OpenGLStagingBuffer>(size);
    }

    RHI::RHIGraphicsPipelineStateSP OpenGLRHIApi::CreateGraphicsPipelineState(const RHI::RHIGraphicsPipelineStateDesc& desc)
    {
        return std::make_shared<OpenGLGraphicsPipelineState>(desc);
    }

    RHI::RHIComputePipelineStateSP OpenGLRHIApi::CreateComputePipelineState(const RHI::RHIComputePipelineStateDesc& desc)
    {
        return std::make_shared<OpenGLComputePipelineState>(desc);
    }

    RHI::RHIRayTracingPipelineStateSP OpenGLRHIApi::CreateRayTracingsPipelineState(const RHI::RHIRayTracingPipelineStateDesc& desc)
    {
        return nullptr;
    }

    RHI::RHIVertexDescStateSP OpenGLRHIApi::CreateVertexDescState(const RHI::RHIVertexDescStateDesc& desc)
    {
        return std::make_shared<OpenGLVertexDescState>(desc);
    }

    RHI::RHIRasterizerStateSP OpenGLRHIApi::CreateRasterizerState(const RHI::RHIRasterizerStateDesc& desc)
    {
        return std::make_shared<OpenGLRasterizerState>(desc);
    }

    RHI::RHIColorBlendStateSP OpenGLRHIApi::CreateColorBlendState(const RHI::RHIColorBlendStateDesc& desc)
    {
        return std::make_shared<OpenGLColorBlendState>(desc);
    }

    RHI::RHIDepthStencilStateSP OpenGLRHIApi::CreateDepthStencilState(const RHI::RHIDepthStencilStateDesc& desc)
    {
        return std::make_shared<OpenGLDepthStencilState>(desc);
    }

    RHI::RHIVertexShaderSP OpenGLRHIApi::CreateVertexShader(const std::vector<char>& shaderSourceCode)
    {
        auto shader = std::make_shared<OpenGLVertexShader>();
        shader->Compile(shaderSourceCode);
        return shader;
    }

    RHI::RHIFragmentShaderSP OpenGLRHIApi::CreateFragmentShader(const std::vector<char>& shaderSourceCode)
    {
        auto shader = std::make_shared<OpenGLFragmentShader>();
        shader->Compile(shaderSourceCode);
        return shader;
    }

    RHI::RHIComputeShaderSP OpenGLRHIApi::CreateComputeShader(const std::vector<char>& shaderSourceCode)
    {
        auto shader = std::make_shared<OpenGLComputeShader>();
        shader->Compile(shaderSourceCode);
        return shader;
    }

    RHI::RHIGeometryShaderSP OpenGLRHIApi::CreateGeometryShader(const std::vector<char>& shaderSourceCode)
    {
        auto shader = std::make_shared<OpenGLGeometryShader>();
        shader->Compile(shaderSourceCode);
        return shader;
    }

    RHI::RHITessControlShaderSP OpenGLRHIApi::CreateTessControlShader(const std::vector<char>& shaderSourceCode)
    {
        auto shader = std::make_shared<OpenGLTessControlShader>();
        shader->Compile(shaderSourceCode);
        return shader;
    }

    RHI::RHITessEvalShaderSP OpenGLRHIApi::CreateTessEvalShader(const std::vector<char>& shaderSourceCode)
    {
        auto shader = std::make_shared<OpenGLTessEvalShader>();
        shader->Compile(shaderSourceCode);
        return shader;
    }

    RHI::RHIMeshShaderSP OpenGLRHIApi::CreateMeshShader(const std::vector<char>& shaderSourceCode)
    {
        return nullptr;
    }

    RHI::RHITaskShaderSP OpenGLRHIApi::CreateTaskShader(const std::vector<char>& shaderSourceCode)
    {
        return nullptr;
    }

    RHI::RHIRayGenShaderSP OpenGLRHIApi::CreateRayGenShader(const std::vector<char>& shaderSourceCode)
    {
        return nullptr;
    }

    RHI::RHICloseHitShaderSP OpenGLRHIApi::CreateCloseHitShader(const std::vector<char>& shaderSourceCode)
    {
        return nullptr;
    }

    RHI::RHIMissShaderSP OpenGLRHIApi::CreateMissShader(const std::vector<char>& shaderSourceCode)
    {
        return nullptr;
    }

    RHI::RHIAnyHitShaderSP OpenGLRHIApi::CreateAnyHitShader(const std::vector<char>& shaderSourceCode)
    {
        return nullptr;
    }

    RHI::RHIIntersectionShaderSP OpenGLRHIApi::CreateIntersectionShader(const std::vector<char>& shaderSourceCode)
    {
        return nullptr;
    }

    RHI::RHICallableShaderSP OpenGLRHIApi::CreateCallableShader(const std::vector<char>& shaderSourceCode)
    {
        return nullptr;
    }

    RHI::RHISwapchainSP OpenGLRHIApi::CreateSwapchain(void* inWindowHandle, uint32_t w, uint32_t h, RHI::ERHIFormat format)
    {
        return std::make_shared<OpenGLSwapchain>(inWindowHandle, w, h, format);
    }

    RHI::RHISamplerSP OpenGLRHIApi::CreateSampler(const RHI::RHISamplerDesc& desc)
    {
        return std::make_shared<OpenGLSampler>(desc);
    }

    RHI::RHIQueue* OpenGLRHIApi::GetQueue(RHI::EQueueType Type)
    {
        static OpenGLQueue graphicsQueue(RHI::EQueueType::Graphics);
        static OpenGLQueue computeQueue(RHI::EQueueType::Compute);

        switch (Type)
        {
        case RHI::EQueueType::Graphics:
            return &graphicsQueue;
        case RHI::EQueueType::Compute:
            return &computeQueue;
        default:
            return &graphicsQueue;
        }
    }

    RHI::RHIPresentExecutor* OpenGLRHIApi::GetPresentExecutor()
    {
        static OpenGLPresentExecutor executor(dynamic_cast<OpenGLQueue*>(GetQueue(RHI::EQueueType::Graphics)));
        return &executor;
    }

    void OpenGLRHIApi::RHICreateTransition(RHI::RHITransition* Transition, const RHI::RHITransitionCreateInfo& CreateInfo)
    {
    }

    void OpenGLRHIApi::RHIReleaseTransition(RHI::RHITransition* Transition)
    {
    }

    RHI::RHITransientResourceManagerSP OpenGLRHIApi::CreateTransientResourceManager()
    {
        return std::make_shared<OpenGLTransientResourceManager>();
    }

    OpenGLRHIModule::OpenGLRHIModule() = default;
    OpenGLRHIModule::~OpenGLRHIModule() = default;

    void OpenGLRHIModule::StartupModule()
    {
        RHI::GRHIApi = CreateRHIApi();
        RHI::GRHIApi->Init();
        bLoaded = true;
    }

    void OpenGLRHIModule::ShutdownModule()
    {
        delete RHI::GRHIApi;
        RHI::GRHIApi = nullptr;
        bLoaded = false;
    }

    bool OpenGLRHIModule::IsLoaded() const
    {
        return bLoaded;
    }

    RHI::RHIApi* OpenGLRHIModule::CreateRHIApi()
    {
        return new OpenGLRHIApi();
    }

    IMPLEMENT_SIMPLE_MODULE(OpenGLRHIModule, "RHIOpenGL");
}
