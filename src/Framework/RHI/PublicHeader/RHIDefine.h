#pragma once
#include <vector>
#include <unordered_map>
#include <string>
// 一个简易版的 UE 风格宏
#define ENUM_CLASS_FLAGS(Enum) \
    inline Enum operator|(Enum a, Enum b) { return static_cast<Enum>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); } \
    inline Enum& operator|=(Enum& a, Enum b) { a = a | b; return a; } \
    inline Enum operator&(Enum a, Enum b) { return static_cast<Enum>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); } \
    inline bool EnumHasAnyFlags(Enum Flags, Enum Contains) { return (static_cast<uint32_t>(Flags) & static_cast<uint32_t>(Contains)) != 0; }



namespace RHI
{

    enum class EShaderUniformBaseType {
        Unknown = 0,
        Float32,
        Int32,
        UInt32,
        Bool,
        // 可根据需要扩展更多类型
        Texture,
        Texture_SRV,
        Texture_UAV,
        Buffer,
        Buffer_SRV,
        Buffer_UAV,
        Sampler,
        ColorBindings,
        //
        Struct//结构体类型，内部可以含有其他类型数据
    };

// 常

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
    R32G32B32_Float,
    R32G32B32A32_Float,
    D24_UNorm_S8_UInt,
    D32_Float,
    // ...可根据需要扩展
};

struct RHI_API FormatInfo
{
    std::string Name; // 名称，如 "R8G8B8A8_UNorm"
    uint32_t BytesPerPixel = 0; // 每像素字节数
    uint32_t NumComponents = 0; // 分量数量（R,G,B,A）
    bool bIsDepth = false; // 是否深度纹理
    bool bIsStencil = false; // 是否模板纹理
    bool bIsSRGB = false; // 是否 sRGB
    bool bIsFloat = false; // 是否浮点格式
};

extern RHI_API const std::unordered_map<ERHIFormat, FormatInfo> GFormatInfoMap;


// 资源访问类型
enum class ERHIResourceAccess
{
    Unknown = 0,
    Undefined = 1 << 0,  // 初始状态，不保留旧内容 (Discard)

    // --- 只读语义 (Read-only) ---
    CPURead = 1 << 1,  // CPU 读回
    Present = 1 << 2,  // 交换链呈现 (Swapchain Read)
    IndirectArgs = 1 << 3,  // 间接命令参数 (Indirect Draw/Dispatch)
    VertexOrIndexBuffer = 1 << 4, // 顶点或索引缓冲区读取
    SRVGraphics = 1 << 5,  // 图形着色器 (VS/PS/etc.) 采样或读取
    SRVCompute = 1 << 6,  // 计算着色器读取
    CopySrc = 1 << 7,  // 拷贝操作的源 (Transfer Src)
    ResolveSrc = 1 << 8,  // 多重采样 Resolve 的源
    DSVRead = 1 << 9,  // 深度/模板只读测试 (Depth Read Only)
    ShadingRateSource = 1 << 10, // 可变速率着色掩码图

    // --- 读写语义 (Read-Write) ---
    UAVGraphics = 1 << 11, // 图形管线随机读写 (Storage Image/Buffer)
    UAVCompute = 1 << 12, // 计算管线随机读写
    RTV = 1 << 13, // 颜色附件写入
    CopyDest = 1 << 14, // 拷贝操作的目的 (Transfer Dst)
    ResolveDst = 1 << 15, // 多重采样 Resolve 的目的
    DSVWrite = 1 << 16, // 深度/模板写入

    // --- 光线追踪 (Ray Tracing) ---
    BVHRead = 1 << 17, // 加速结构读取 (TraceRay/Build Input)
    BVHWrite = 1 << 18, // 加速结构构建写入

    // --- 掩码与常用组合 ---
    SRVMask = SRVGraphics | SRVCompute,
    UAVMask = UAVGraphics | UAVCompute,

    // 排他性只读掩码（这些状态通常不与写入状态并存）
    ReadOnlyExclusiveMask = CPURead | Present | IndirectArgs | VertexOrIndexBuffer | SRVMask | CopySrc | ResolveSrc | BVHRead,

    // 可读状态掩码（包含 UAV）
    ReadableMask = ReadOnlyExclusiveMask | DSVRead | UAVMask,

    // 可写状态掩码
    WritableMask = RTV | UAVMask | DSVWrite | CopyDest | ResolveDst | BVHWrite
};
// 使用宏
ENUM_CLASS_FLAGS(ERHIResourceAccess);

enum class ERHIFilter { Nearest, Linear };
enum class ERHIAddressMode { Repeat, ClampToEdge, MirrorClampToEdge, MirrorRepeat };

// 资源类型
enum class ERHIResourceType
{
    Unknown = 0,
    Texture,
    Buffer,
    UniformBuffer,
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
enum class ERHIShaderFrequency
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

    //定义color结构体
    struct RHI_API RHIColor{
        float r, g, b, a;
    };

    struct RHI_API RHIRect
    {
        int32_t X, Y;
        uint32_t Width, Height;

        RHIRect(int32_t InX = 0, int32_t InY = 0, uint32_t InWidth = 0, uint32_t InHeight = 0)
            : X(InX), Y(InY), Width(InWidth), Height(InHeight) {}

        // 可扩展其他方法，如计算面积、中心点等
    };

    //纹理相关描述
    // 纹理类型枚举
    enum class ERHITextureType
    {
		Unknown = 0,
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
        Texture2DArray,
        TextureCubeArray
    };

    // 纹理用途标志（可组合使用）
    enum class ERHITextureCreateFlags : uint32_t
    {
        None = 0,
        ShaderResource = 1ull << 0,  // 可作为采样贴图 (SRV)
        RenderTarget = 1ull << 1,  // 可作为颜色附件 (RTV)
        DepthStencil = 1ull << 2,  // 可作为深度附件 (DSV)
        UAV = 1ull << 3,  // 可随机读写 (Storage Image)
        Presentable = 1ull << 4,  // 可用于显示输出 (Swapchain)

        // 内存与更新属性
        Dynamic = 1ull << 5,  // 频繁更新 (Hint)
        CPUReadback = 1ull << 6,  // CPU 需读取 (Host Visible)
        Memoryless = 1ull << 7,  // 仅存在于 Tile Memory (手机端优化)

        // 拷贝属性
        CopySrc = 1ull << 8,  // 可作为拷贝源
        CopyDest = 1ull << 9,  // 可作为拷贝目的
    };
    // 使用宏
    ENUM_CLASS_FLAGS(ERHITextureCreateFlags);

    // 纹理描述结构体
    struct RHI_API RHITextureDesc
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
        ERHITextureCreateFlags Usage = ERHITextureCreateFlags::None; // 纹理用途
        bool bGenerateMips = false;          // 是否生成Mip贴图
        bool bCPUAccessible = false;         // CPU是否可访问
        const void* InitialData = nullptr;   // 初始数据
        const char* DebugName = nullptr;     // 调试名称

    };

    // 纹理视图描述
    struct RHI_API RHITextureViewDesc
    {
        ERHIFormat Format = ERHIFormat::Unknown; // 视图格式
        uint32_t MipLevel = 0;               // 起始Mip级别
        uint32_t MipLevelCount = 1;          // Mip级别数量
        uint32_t ArraySlice = 0;             // 起始数组切片
        uint32_t ArraySliceCount = 1;        // 数组切片数量
        ERHITextureCreateFlags Flags = ERHITextureCreateFlags::ShaderResource; // 视图用途
    };



    //缓冲区相关描述

    // 缓冲区用途标志（可组合使用）
    enum class ERHIBufferUsageFlags
    {
        None = 0,
        // ---------- Buffer 类型 ----------
        Vertex = 1 << 0,  // 顶点缓冲区
        Index = 1 << 1,  // 索引缓冲区
        Constant = 1 << 2,  // 常量缓冲区 (Uniform)
        Structured = 1 << 3,  // 结构化缓冲区
        RawBuffer = 1 << 4,  // 原始缓冲区
        Indirect = 1 << 5,  // 间接绘制缓冲区
        Staging = 1 << 6,  // 暂存缓冲区（CPU->GPU 上传）
        Texel = 1 << 7,   // 
        // ---------- Buffer 使用方式 ----------
        ShaderResource = 1 << 16, // 作为 SRV
        UnorderedAccess = 1 << 17, // 作为 UAV
        TransferSrc = 1 << 18, // 传输源
        TransferDst = 1 << 19, // 传输目标
    };
    // 使用宏
    ENUM_CLASS_FLAGS(ERHIBufferUsageFlags);

    // 缓冲区描述结构体
    struct RHI_API RHIBufferDesc
    {
        uint64_t Size = 0;                   // 缓冲区大小（字节）
        uint32_t Stride = 0;                 // 结构化缓冲区的元素大小
        ERHIBufferUsageFlags Usage = ERHIBufferUsageFlags::None; // 缓冲区用途
        bool bCPUAccessible = false;         // CPU是否可访问
        const char* DebugName = nullptr;     // 调试名称
    };

    // 缓冲区视图描述
    struct RHI_API RHIBufferViewDesc
    {
        uint64_t Offset = 0;                 // 视图起始偏移
        uint64_t Size = 0;                   // 视图大小
        ERHIBufferUsageFlags Usage = ERHIBufferUsageFlags::None; // 视图用途
    };


    //采样器相关描述
    struct RHI_API RHISamplerDesc
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

    struct RHI_API RHIVertexBindingDesc
    {
        uint32_t binding = 0;
        uint32_t stride = 0;
        ERHIInputRate inputRate = ERHIInputRate::PerVertex;
    };

    struct RHI_API RHIVertexAttributeDesc
    {
        uint32_t location = 0;
        uint32_t binding = 0;       
        uint32_t offset = 0;
        ERHIFormat format = ERHIFormat::Unknown; // 可用ERHIFormat或自定义格式
    };

    struct RHI_API RHIVertexDescStateDesc
    {
        std::vector<RHIVertexBindingDesc> bindings;
        std::vector<RHIVertexAttributeDesc> attributes;
    };

    enum class ERHIPolygonMode { Fill, Line, Point };
    enum class ERHICullMode { None, Front, Back, FrontAndBack };
    enum class ERHIFrontFace { CounterClockwise, Clockwise };

    struct RHI_API RHIRasterizerStateDesc
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


    struct RHI_API RHIColorBlendAttachmentDesc
    {
        bool blendEnable = false;
        uint32_t colorWriteMask = 0xF; // RGBA
        // 可扩展BlendFactor/BlendOp等
    };

    struct RHI_API RHIColorBlendStateDesc
    {
        bool logicOpEnable = false;
        std::vector<RHIColorBlendAttachmentDesc> attachments;
        float blendConstants[4] = {0, 0, 0, 0};
        // 可扩展logicOp
    };

    enum class ERHICompareOp { Never, Less, Equal, LessOrEqual, Greater, NotEqual, GreaterOrEqual, Always };


    struct RHI_API RHIDepthStencilStateDesc
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

    // For transitions
    enum class ERHIPipeline : uint8_t
    {
        None = 0,
        Graphics = 1 << 0,
        AsyncCompute = 1 << 1,
        Num = 2
    };
    ENUM_CLASS_FLAGS(ERHIPipeline)

    enum class EResourceTransitionFlags : uint8_t
    {
        None = 0,
        MaintainCompression = 1 << 0,
        Discard = 1 << 1,
    };
    ENUM_CLASS_FLAGS(EResourceTransitionFlags)

    enum class ERHITransitionCreateFlags : uint8_t
    {
        None = 0,
        NoSplit = 1 << 0,
    };
    ENUM_CLASS_FLAGS(ERHITransitionCreateFlags)

    // 类似 UE 的 EPrimitiveType，用于描述顶点绘制拓扑
    enum class EPrimitiveTopology : uint8_t
    {
        PointList,                   // 点列表，对应 VK_PRIMITIVE_TOPOLOGY_POINT_LIST
        LineList,                    // 线段列表，对应 VK_PRIMITIVE_TOPOLOGY_LINE_LIST
        LineStrip,                   // 线段条，对应 VK_PRIMITIVE_TOPOLOGY_LINE_STRIP
        TriangleList,                // 三角形列表，对应 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        TriangleStrip,               // 三角形条，对应 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
        TriangleFan,                 // 三角形扇，对应 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN
        LineListWithAdjacency,       // 带邻接信息的线段列表，对应 VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY
        LineStripWithAdjacency,      // 带邻接信息的线段条，对应 VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY
        TriangleListWithAdjacency,   // 带邻接信息的三角形列表，对应 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY
        TriangleStripWithAdjacency,  // 带邻接信息的三角形条，对应 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY
        PatchList_1,                 // Patch 列表，1 个控制点，对应 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
        PatchList_2,                 // Patch 列表，2 个控制点
        PatchList_3,                 // Patch 列表，3 个控制点
        PatchList_4,                 // Patch 列表，4 个控制点，可继续扩展更多
        Unknown                       // 未知类型
    };



    struct RHI_API RHIViewportDesc
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

    struct RHI_API RHITextureRegion{
        uint32_t mipLevel = 0; // Mip级别
        uint32_t arraySlice = 0; // 数组切片
        uint32_t numMipLevels = 1; // 更新区域Mip级别数量
        uint32_t numArraySlices = 1; // 更新区域数组切片数量
        uint32_t xOffset = 0; // X偏移
        uint32_t yOffset = 0; // Y偏移
        uint32_t zOffset = 0; // Z偏移
        uint32_t width = 0; // 更新区域宽度
        uint32_t height = 0; // 更新区域高度
        uint32_t depth = 1; // 更新区域深度（3D纹理）
        static RHITextureRegion Create2DRegion(uint32_t InWidth,uint32_t InHeight) {
			RHITextureRegion region;
			region.mipLevel = 0;
			region.arraySlice = 0;
			region.numMipLevels = 1;
			region.numArraySlices = 1;
			region.xOffset = 0;
			region.yOffset = 0;
			region.zOffset = 0;
			region.width = InWidth;
			region.height = InHeight;
			region.depth = 1;
			return region;
        }
    };
    struct RHI_API RHIBufferRegion {
        uint32_t offset = 0; // 偏移
        uint32_t size = 0; // 大小
    };

    // ---------------------------
// Texture Shader Resource View
// ---------------------------
    struct RHI_API RHITexSRVCreateInfo
    {
        uint32_t MostDetailedMip = 0;
        uint32_t MipLevelCount = 1;
        uint32_t FirstArraySlice = 0;
        uint32_t ArraySize = 1;
        ERHIFormat Format = ERHIFormat::Unknown;
    };

    // ---------------------------
    // Texture Unordered Access View
    // ---------------------------
    struct RHI_API RHITexUAVCreateInfo
    {
        uint32_t MipSlice = 0;
        uint32_t FirstArraySlice = 0;
        uint32_t ArraySize = 1;
        ERHIFormat Format = ERHIFormat::Unknown;
    };

    // ---------------------------
    // Buffer Shader Resource View
    // ---------------------------
    struct RHI_API RHIBufferSRVCreateInfo
    {
        uint64_t Offset = 0;        // 起始字节偏移
        uint64_t NumElements = 0;   // 元素数量
        uint32_t Stride = 0;        // 每个元素字节数
        ERHIFormat Format = ERHIFormat::Unknown;
    };

    // ---------------------------
    // Buffer Unordered Access View
    // ---------------------------
    using RHIBufferUAVCreateInfo = RHIBufferSRVCreateInfo;


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

    struct RHIClearValueBinding
    {
        enum class ClearValueBinding
        {
            None,          // 不需要 clear
            Color,         // ClearColor
            DepthStencil   // ClearDepth / ClearStencil
        } Binding;
        union
        {
            float Color[4];
            struct { float Depth; uint32_t Stencil; };
        };

    };

    struct RHICopyTextureDesc {
        // 源 mip / array slice
        uint32_t SrcMipIndex = 0;
        uint32_t SrcArraySlice = 0;

        // 目标 mip / array slice
        uint32_t DstMipIndex = 0;
        uint32_t DstArraySlice = 0;
    };


    struct RHIUpdateTextureRegion2D
    {
        uint32_t DestX = 0; // 目标纹理起始 X
        uint32_t DestY = 0; // 目标纹理起始 Y
        uint32_t SrcX = 0; // 源数据起始 X
        uint32_t SrcY = 0; // 源数据起始 Y
        uint32_t Width = 0; // 更新宽度
        uint32_t Height = 0; // 更新高度


        RHIUpdateTextureRegion2D() = default;


        RHIUpdateTextureRegion2D(uint32_t InDestX, uint32_t InDestY,
            uint32_t InSrcX, uint32_t InSrcY,
            uint32_t InWidth, uint32_t InHeight)
            : DestX(InDestX), DestY(InDestY),
            SrcX(InSrcX), SrcY(InSrcY),
            Width(InWidth), Height(InHeight)
        {
        }
    };



    struct RHIUpdateTextureRegion3D
    {
        uint32_t DestX = 0;
        uint32_t DestY = 0;
        uint32_t DestZ = 0;
        uint32_t SrcX = 0;
        uint32_t SrcY = 0;
        uint32_t SrcZ = 0;
        uint32_t Width = 0;
        uint32_t Height = 0;
        uint32_t Depth = 0;


        RHIUpdateTextureRegion3D() = default;


        RHIUpdateTextureRegion3D(uint32_t InDestX, uint32_t InDestY, uint32_t InDestZ,
            uint32_t InSrcX, uint32_t InSrcY, uint32_t InSrcZ,
            uint32_t InWidth, uint32_t InHeight, uint32_t InDepth)
            : DestX(InDestX), DestY(InDestY), DestZ(InDestZ),
            SrcX(InSrcX), SrcY(InSrcY), SrcZ(InSrcZ),
            Width(InWidth), Height(InHeight), Depth(InDepth)
        {
        }
    };

    enum class ERenderTargetLoadOp : uint8_t
    {
        DontCare = 0,   // 不保留旧内容
        Load = 1,   // 读取旧内容
        Clear = 2    // Clear
    };

    enum class ERenderTargetStoreOp : uint8_t
    {
        DontCare = 0,   // 不保留结果
        Store = 1,   // 保存结果
        Resolve = 2    // MSAA Resolve（代替 Store）
    };

    enum class ERenderTargetActions : uint8_t
    {
        // 低 2 bit = StoreOp
        // 高 2 bit = LoadOp
        LoadOpShift = 2,
        StoreOpMask = 0x3,

#define RT_ACTION_MAKE(Load, Store) \
    ( (uint8_t(ERenderTargetLoadOp::Load)  << uint8_t(LoadOpShift)) | \
      (uint8_t(ERenderTargetStoreOp::Store)) )

        DontCare_DontCare = RT_ACTION_MAKE(DontCare, DontCare),

        DontCare_Store = RT_ACTION_MAKE(DontCare, Store),
        Clear_Store = RT_ACTION_MAKE(Clear, Store),
        Load_Store = RT_ACTION_MAKE(Load, Store),

        Clear_DontCare = RT_ACTION_MAKE(Clear, DontCare),
        Load_DontCare = RT_ACTION_MAKE(Load, DontCare),

        Clear_Resolve = RT_ACTION_MAKE(Clear, Resolve),
        Load_Resolve = RT_ACTION_MAKE(Load, Resolve),

#undef RT_ACTION_MAKE
    };

#define MAX_RENDER_TARGETS 8
}