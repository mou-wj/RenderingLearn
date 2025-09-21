#pragma once
#include <vector>

namespace RHI
{

// 常用的图形格式
enum class ERHIFormat
{
    Unknown = 0,
    R8_UNorm,
    R8G8B8_UNorm,
    R8G8B8_SRGB,
    B8G8R8_UNorm,
    B8G8R8_SRGB,
    R8G8B8A8_UNorm,
    R8G8B8A8_SRGB,
    B8G8R8A8_UNorm,
    B8G8R8A8_SRGB,
    R16G16_Float,
    R16G16B16A16_Float,
    R32_Float,
    R32G32_Float,
    R32G32B32A32_Float,
    D24_UNorm_S8_UInt,
    D32_Float,
    // ...可根据需要扩展
};


// 资源访问类型
enum class ERHIResourceAccess
{
    Unknown = 0,
    Read,
    Write,
    ReadWrite,
    // ...可根据需要扩展
};

enum class ERHIFilter { Nearest, Linear };
enum class ERHIAddressMode { Repeat, ClampToEdge, MirrorClampToEdge, MirrorRepeat };

// 资源类型
enum class ERHIResourceType
{
    Unknown = 0,
    Texture,
    Buffer,
    GraphicPipelineState,
    ComputePipelineState,
    RayTracingPipelineState,
    Shader,
    VertexShader,
    FragmentShader,
    GeometryShader,
    ComputeShader,
    TessControlShader,
    TessEvalShader,
    MeshShader,
    TaskShader,
    RayGenShader,
    CloseHitShader,
    MissShader,
    AnyHitShader,
    IntersectionShader,
    CallableShader,
    VertexDescState,
    RasterizerState,
    ColorBlendState,
    DepthStencilState,
    Fence,
    Viewport,
    Sampler,
    // ...可扩展
    ShaderResourceView,
    UnorderedAccessView
};


// 着色器平台类型
enum class ERHIShaderPlatform
{
    Unknown = 0,
    D3D11,
    D3D12,
    Vulkan,
    OpenGL,
    Metal,
    Switch,
    PS5,
    Xbox,
    // ...可扩展
};

// 着色器类型
enum class ERHIShaderType
{
    Unknown = 0,
    Vertex,
    Fragment,
    Geometry,
    Compute,
    TessControl,
    TessEvaluation,
    Mesh,
    Task,
    RayGen,
    ClosestHit,
    Miss,
    AnyHit,
    Intersection,
    Callable,
    // ...可扩展
};


    /** 加载操作类型 */
    enum class ERHILoadAction : uint8_t
    {
        Load,     // 保留现有内容
        Clear,    // 清除内容
        DontCare  // 不关心现有内容
    };
    
    /** 存储操作类型 */
    enum class ERHIStoreAction : uint8_t
    {
        Store,                   // 存储内容
        MultisampleResolve,      // 多重采样解析
        StoreAndMultisampleResolve, // 存储并解析多重采样
        DontCare                // 不关心存储结果
    };

    //定义color结构体
    struct RHIColor{
        float r, g, b, a;
    };

    struct RHIIntRect
    {
        int32_t X, Y, Width, Height;

        RHIIntRect(int32_t InX = 0, int32_t InY = 0, int32_t InWidth = 0, int32_t InHeight = 0)
            : X(InX), Y(InY), Width(InWidth), Height(InHeight) {}

        // 可扩展其他方法，如计算面积、中心点等
    };

    //纹理相关描述
    // 纹理类型枚举
    enum class ERHITextureType
    {
        Texture2D,
        Texture3D,
        TextureCube,
        Texture2DArray,
        TextureCubeArray
    };

    // 纹理用途标志（可组合使用）
    enum ERHITextureFlags
    {
        None = 0,
        ShaderResource = 1 << 0,
        RenderTarget = 1 << 1,
        UnorderedAccess = 1 << 2,
        DepthStencil = 1 << 3,
        TransferSrc = 1 << 4,
        TransferDst = 1 << 5
    };

    // 纹理描述结构体
    struct RHITextureDesc
    {
        uint32_t Width = 1;                  // 纹理宽度
        uint32_t Height = 1;                 // 纹理高度
        uint32_t Depth = 1;                  // 纹理深度（3D纹理）
        uint32_t MipLevels = 1;              // Mip层级数量
        uint32_t ArraySize = 1;              // 数组大小
        ERHIFormat Format = ERHIFormat::Unknown; // 像素格式
        ERHITextureType Type = ERHITextureType::Texture2D;   // 纹理类型
        uint32_t SampleCount = 1;            // 多重采样数量
        uint32_t SampleQuality = 0;          // 多重采样质量
        uint32_t Usage = ERHITextureFlags::ShaderResource; // 纹理用途
        bool bGenerateMips = false;          // 是否生成Mip贴图
        bool bCPUAccessible = false;         // CPU是否可访问
        const void* InitialData = nullptr;   // 初始数据
        const char* DebugName = nullptr;     // 调试名称
    };

    // 纹理视图描述
    struct RHITextureViewDesc
    {
        ERHIFormat Format = ERHIFormat::Unknown; // 视图格式
        uint32_t MipLevel = 0;               // 起始Mip级别
        uint32_t MipLevelCount = 1;          // Mip级别数量
        uint32_t ArraySlice = 0;             // 起始数组切片
        uint32_t ArraySliceCount = 1;        // 数组切片数量
        uint32_t Flags = ERHITextureFlags::ShaderResource; // 视图用途
    };


    //缓冲区相关描述
        // 缓冲区类型枚举
    enum class ERHIBufferType
    {
        Vertex,         // 顶点缓冲区
        Index,          // 索引缓冲区
        Constant,       // 常量缓冲区
        Structured,     // 结构化缓冲区
        RawBuffer,      // 原始缓冲区
        Indirect,       // 间接绘制缓冲区
        Staging         // 暂存缓冲区（用于CPU到GPU的数据传输）
    };

    // 缓冲区用途标志（可组合使用）
    enum class ERHIBufferFlags
    {
        None = 0,
        ShaderResource = 1 << 0,
        UnorderedAccess = 1 << 1,
        TransferSrc = 1 << 2,
        TransferDst = 1 << 3,
        MapRead = 1 << 4,
        MapWrite = 1 << 5
    };

    // 缓冲区描述结构体
    struct RHIBufferDesc
    {
        uint64_t Size = 0;                   // 缓冲区大小（字节）
        uint32_t Stride = 0;                 // 结构化缓冲区的元素大小
        ERHIBufferType Type = ERHIBufferType::Vertex; // 缓冲区类型
        uint32_t Usage = (uint32_t)ERHIBufferFlags::ShaderResource; // 缓冲区用途
        bool bCPUAccessible = false;         // CPU是否可访问
        const void* InitialData = nullptr;   // 初始数据
        const char* DebugName = nullptr;     // 调试名称
    };

    // 缓冲区视图描述
    struct RHIBufferViewDesc
    {
        uint64_t Offset = 0;                 // 视图起始偏移
        uint64_t Size = 0;                   // 视图大小
        uint32_t Flags = (uint32_t)ERHIBufferFlags::ShaderResource; // 视图用途
    };


    //采样器相关描述
    struct RHISamplerDesc
    {
        ERHIFilter magFilter = ERHIFilter::Linear;
        ERHIFilter minFilter = ERHIFilter::Linear;
        ERHIAddressMode addressU = ERHIAddressMode::Repeat;
        ERHIAddressMode addressV = ERHIAddressMode::Repeat;
        ERHIAddressMode addressW = ERHIAddressMode::Repeat;
        float mipLodBias = 0.0f;
        float minLod = 0.0f;
        float maxLod = 1000.0f;
        bool anisotropyEnable = false;
        float maxAnisotropy = 1.0f;
        // 可扩展border color、compare op等
    };



    //管线状态描述
    enum class ERHIInputRate { PerVertex, PerInstance };

    struct RHIVertexBindingDesc
    {
        uint32_t binding = 0;
        uint32_t stride = 0;
        ERHIInputRate inputRate = ERHIInputRate::PerVertex;
    };

    struct RHIVertexAttributeDesc
    {
        uint32_t location = 0;
        uint32_t offset = 0;
        ERHIFormat format = ERHIFormat::Unknown; // 可用ERHIFormat或自定义格式
    };

    struct RHIVertexDescStateDesc
    {
        std::vector<RHIVertexBindingDesc> bindings;
        std::vector<RHIVertexAttributeDesc> attributes;
    };

    enum class ERHIPolygonMode { Fill, Line, Point };
    enum class ERHICullMode { None, Front, Back, FrontAndBack };
    enum class ERHIFrontFace { CounterClockwise, Clockwise };

    struct RHIRasterizerStateDesc
    {
        bool depthClampEnable = false;
        bool rasterizerDiscardEnable = false;
        ERHIPolygonMode polygonMode = ERHIPolygonMode::Fill;
        ERHICullMode cullMode = ERHICullMode::Back;
        ERHIFrontFace frontFace = ERHIFrontFace::CounterClockwise;
        bool depthBiasEnable = false;
        float depthBiasConstantFactor = 0.0f;
        float depthBiasClamp = 0.0f;
        float depthBiasSlopeFactor = 0.0f;
        float lineWidth = 1.0f;
    };


    struct RHIColorBlendAttachmentDesc
    {
        bool blendEnable = false;
        uint32_t colorWriteMask = 0xF; // RGBA
        // 可扩展BlendFactor/BlendOp等
    };

    struct RHIColorBlendStateDesc
    {
        bool logicOpEnable = false;
        std::vector<RHIColorBlendAttachmentDesc> attachments;
        float blendConstants[4] = {0, 0, 0, 0};
        // 可扩展logicOp
    };

    enum class ERHICompareOp { Never, Less, Equal, LessOrEqual, Greater, NotEqual, GreaterOrEqual, Always };


    struct RHIDepthStencilStateDesc
    {
        bool depthTestEnable = false;
        bool depthWriteEnable = false;
        ERHICompareOp depthCompareOp = ERHICompareOp::Less;
        bool depthBoundsTestEnable = false;
        bool stencilTestEnable = false;
        // 可扩展stencilOpState等
        uint32_t stencilReadMask = 0;
        uint32_t stencilWriteMask = 0;
        
    };

    enum class ERHIPipelineType
    {
        Unknown = 0,
        Graphics,
        Compute,
        RayTracing
    };




    struct RHIViewportDesc
    {
		float x = 0.0f; // 左上角X坐标
		float y = 0.0f; // 左上角Y坐标
		float width = 800.0f; // 视口宽度
		float height = 600.0f; // 视口高度
		float minDepth = 0.0f; // 最小深度
		float maxDepth = 1.0f; // 最大深度
        uint32_t imageCount = 2; // swapchain image数量
        // 可扩展像素格式、VSync等
    };

    struct RHITextureRegion{
        uint32_t mipLevel = 0; // Mip级别
        uint32_t arraySlice = 0; // 数组切片
        uint32_t xOffset = 0; // X偏移
        uint32_t yOffset = 0; // Y偏移
        uint32_t zOffset = 0; // Z偏移
        uint32_t width = 0; // 更新区域宽度
        uint32_t height = 0; // 更新区域高度
        uint32_t depth = 1; // 更新区域深度（3D纹理）
    };


    enum class EVerdorId
    {
        Unknown = 0,
        AMD = 0x1002,
        Intel = 0x8086,
        NVIDIA = 0x10DE,
        ARM = 0x13B5,
        Qualcomm = 0x5143,
        Apple = 0x106B,
        Microsoft = 0x1414

	};
}