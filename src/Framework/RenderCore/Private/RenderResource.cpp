#include "RenderResource.h"
#include "RHIApi.h"
#include "AssetManager.h"
#include "stb_image.h"
#include "RenderThread.h"
using namespace RHI;
namespace RenderCore {

// RenderResource
RenderResource::RenderResource() = default;
RenderResource::~RenderResource() = default;

void RenderResource::InitRHIResource() {
    bInitialized = true;
}

void RenderResource::ReleaseRHIResource() {
    bInitialized = false;
}

void RenderResource::SetName(const std::string& name) {
    Name = name;
}

const std::string& RenderResource::GetName() const {
    return Name;
}

bool RenderResource::IsInitialized() const {
    return bInitialized.load();
}

// RenderTexture
RenderTexture::RenderTexture(const RHI::RHITextureDesc& inDesc) : Desc(inDesc) {}
RenderTexture::~RenderTexture() = default;

void RenderTexture::InitRHIResource() {
	Texture = RHI::GRHIApi->CreateTexture(Desc);
    RenderResource::InitRHIResource();
    // 纹理资源初始化逻辑
}

void RenderTexture::ReleaseRHIResource() {
    // 纹理资源释放逻辑
	Texture.reset();
	Texture = nullptr;
    RenderResource::ReleaseRHIResource();
}

// RenderBuffer
RenderBuffer::RenderBuffer(const RHI::RHIBufferDesc& inDesc) : Desc(inDesc) {};
RenderBuffer::~RenderBuffer() = default;

void RenderBuffer::InitRHIResource() {
	Buffer = RHI::GRHIApi->CreateBuffer(Desc);	
    RenderResource::InitRHIResource();
    // 缓冲区资源初始化逻辑
}

void RenderBuffer::ReleaseRHIResource() {
    // 缓冲区资源释放逻辑
	Buffer.reset();
	Buffer = nullptr;
    RenderResource::ReleaseRHIResource();
}




RenderTargetPool* GRenderTargetPool = nullptr;


TransientResourceAllocator* GTransientResourceAllocator = nullptr;

// 分配或复用 RenderTarget（重命名GetFreeRenderTarget，增加垃圾回收和分配管理）
std::shared_ptr<PooledRenderTarget> RenderTargetPool::GetFreeRenderTarget(
	const PoolRenderTargetDesc& Desc)
{
	std::lock_guard<std::mutex> Lock(Mutex);
	GarbageCollect();

	if (!FreeList.empty())
	{
		auto RT = FreeList.back();
		FreeList.pop_back();
		AllocatedList.push_back(RT);
		return RT;
	}
	auto descRHI = PoolRenderTargetDesc::ConvertToRHITextureDesc(Desc);
	auto Texture = RHI::GRHIApi->CreateTexture(descRHI);
	// 没有可复用的资源，创建新的
	auto RT = std::make_shared<PooledRenderTarget>(Desc, Texture);
	AllocatedList.push_back(RT);
	return RT;
}
// 垃圾回收：遍历AllocatedList，若target的LastUsedFrame小于当前队列的frame则回收
void RenderTargetPool::GarbageCollect()
{
	auto graphicsQueueFrame = RHI::GRHIApi->GetQueue(EQueueType::Graphics)->GetCurrentTimelineValue(); // 需RHI支持此接口
	auto computeQueueFrame = RHI::GRHIApi->GetQueue(EQueueType::Compute)->GetCurrentTimelineValue(); // 需RHI支持
    auto copyQueueFrame = RHI::GRHIApi->GetQueue(EQueueType::Transfer)->GetCurrentTimelineValue(); // 需RHI


	// 一次遍历，按队列类型和frame判断是否回收
	auto it = AllocatedList.begin();
	while (it != AllocatedList.end()) {
		auto& target = *it;
		auto& tracker = target->GetTracker();
		EQueueType queueType = tracker.GetLastAccessFence().QueueType;
		uint64_t targetFrame = tracker.GetLastAccessFence().Value;
		uint64_t currentFrame = 0;
		switch (queueType) {
			case EQueueType::Graphics:
				currentFrame = graphicsQueueFrame;
				break;
			case EQueueType::Compute:
				currentFrame = computeQueueFrame;
				break;
			case EQueueType::Transfer:
				currentFrame = copyQueueFrame;
				break;
			default:
				break;
		}
		if (target && targetFrame < currentFrame) {
			FreeList.push_back(target);
			it = AllocatedList.erase(it);
		} else {
			++it;
		}
	}
}

// 可选：清空池
void RenderTargetPool::Clear()
{
    std::lock_guard<std::mutex> Lock(Mutex);
    FreeList.clear();
}



RenderTexture* GlobalTestTexture = nullptr;

bool InitGlobalRenderResource() {
    auto rootPath = Core::GetProjectDir();
	GlobalTestTexture = CreateTexture(rootPath + "/resources/pic/OIP.jpg");
    return true;
}
void ReleaseGlobalRenderResource() {

}

RenderTexture* CreateTexture(const std::string& Path)
{
	int width, height, channels;

	// 1. 使用 stb_image 加载数据
	// 强制加载为 4 通道 (RGBA)，因为大多数 GPU 对 RGBA 的兼容性比 RGB 更好
	stbi_uc* pixels = stbi_load(Path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{

		return nullptr;
	}
	// 2. 配置纹理描述符
	RHITextureDesc desc;
	desc.Width = static_cast<uint32_t>(width);
	desc.Height = static_cast<uint32_t>(height);
	desc.Depth = 1;
	desc.MipLevels = 1; // 0 通常在 RHI 中表示自动计算全链层级
	desc.ArraySize = 1;

	// 映射格式：因为强制了 STBI_rgb_alpha，这里使用 RGBA8
	// 注意：如果是 UI 图片建议用 RGBA8_UNORM，如果是照片建议用 sRGB
	desc.Format = ERHIFormat::R8G8B8A8_UNorm;

	desc.Type = ERHITextureType::Texture2D;
	desc.SampleCount = 1;
	desc.Usage = ERHITextureCreateFlag::ShaderResource;

	// 如果需要生成 Mips，确保 Usage 包含相应的 Flag (如 RenderTarget 或 UAV，取决于 RHI 实现)
	desc.bGenerateMips = false;

	// 如果后续需要组合更多Usage，请通过 ERHITextureCreateFlagsFlags 组合后再 ToEnum() 赋值。


	desc.InitialData = pixels; // 传入 stbi 的内存指针
	desc.DebugName = Path.c_str();

	RenderCore::RenderTexture* outTexture = nullptr;

	ExecuteSync("Create Texture", [&outTexture, &desc, pixels](RHI::RHICommandListBase& commandList) {

		// 3. 调用 RHI 创建纹理
		RHITextureSP texture = GRHIApi->CreateTexture(desc);
		GRHIApi->UpdateTexture(commandList, texture.get(), pixels, RHITextureRegion::Create2DRegion(desc.Width,desc.Height));
		outTexture = new RenderCore::RenderTexture(desc);
		outTexture->InitRHIResource();
		});

	// 4. 释放 stb_image 分配的 CPU 内存
	stbi_image_free(pixels);
	return outTexture;
}


void TransientResourceAllocator::InitRHI() 
{
	TransientResourceManager = RHI::GRHIApi->CreateTransientResourceManager();
}

void TransientResourceAllocator::ReleaseRHI()
{
	TransientResourceManager.reset();
}

} // namespace RenderCore