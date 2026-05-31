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
RenderTexture::~RenderTexture()
{
	if (Texture) {
		ReleaseRHIResource();
	}
}

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
void RenderTexture::UploadData(const void* data, uint32_t mipIndex, uint32_t arraySlice, uint32_t planeSlice) {
	auto lastQueueType = GetTracker().GetLastAccessFence().QueueType;
	auto* queue = GRHIApi->GetQueue(lastQueueType);
	auto* ctx = queue->AcquireCommandContext();
	RHICommandListBase cmd(ctx);
	cmd.SetImmediate(true);
	cmd.Begin();
	RHI::RHISubresourceRange range;
	range.ArraySlice = arraySlice;
    range.MipIndex = mipIndex;
	range.PlaneSlice = 0;
	auto lastAccess = GetTracker().GetSubresourceAccess(range);
	if (lastAccess != ERHIResourceAccess::TransferDest) {
		std::vector<RHI::RHITransitionInfo> infos;
		infos.emplace_back(Texture.get(), lastAccess, ERHIResourceAccess::TransferDest);
		char* transitionMem = new char[RHI::G_RHITransition_TotalSize];
		auto* transition = new(transitionMem) RHI::RHITransition();
		GRHIApi->RHICreateTransition(transition, RHI::RHITransitionCreateInfo(RHI::ERHITransitionCreateFlags::None, std::move(infos)));

		cmd.BeginTransitions({ transition });
		cmd.EndTransitions({ transition });

		GRHIApi->RHIReleaseTransition(transition);
		delete[] transitionMem;
	}
	RHI::RHIUpdateTextureRegion updateRegion;
	updateRegion.arraySlice = arraySlice;
	updateRegion.mipLevel = mipIndex;
	updateRegion.width = Desc.Width >> mipIndex;
    updateRegion.height = Desc.Height >> mipIndex;

	GRHIApi->UpdateTexture(cmd, Texture.get(), data, updateRegion);
	cmd.End();

	auto fence = queue->ExecuteContext({ ctx }, {});
	GetTracker().UpdateSubresourceAccess(range, ERHIResourceAccess::TransferDest);
	GetTracker().UpdateLastAccessFence(fence);
	queue->WaitFence(fence);
}

// RenderBuffer
RenderBuffer::RenderBuffer(const RHI::RHIBufferDesc& inDesc) : Desc(inDesc) {};
RenderBuffer::~RenderBuffer() = default;

void RenderBuffer::InitRHIResource() {
	Buffer = RHI::GRHIApi->CreateBuffer(Desc);
	RHI::RHIFence fence = {};
	fence.QueueType = Desc.InitialQueueType;
	fence.Value = 0;
    GetTracker().UpdateLastAccessFence(fence);
    RenderResource::InitRHIResource();
    // 缓冲区资源初始化逻辑
}

void RenderBuffer::ReleaseRHIResource() {
    // 缓冲区资源释放逻辑
	Buffer.reset();
	Buffer = nullptr;
    RenderResource::ReleaseRHIResource();
}
void RenderBuffer::UploadData(const void* data, uint32_t size, uint32_t offset) {
	auto lastQueueType = GetTracker().GetLastAccessFence().QueueType;
	auto* queue = GRHIApi->GetQueue(lastQueueType);
	auto* ctx = queue->AcquireCommandContext();
	RHICommandListBase cmd(ctx);
	cmd.SetImmediate(true);
	cmd.Begin();
	if (GetTracker().GetLastAccess() != ERHIResourceAccess::TransferDest) {
		std::vector<RHI::RHITransitionInfo> infos;

		infos.emplace_back(Buffer.get(), GetTracker().GetLastAccess(), ERHIResourceAccess::TransferDest);


		char* transitionMem = new char[RHI::G_RHITransition_TotalSize];
		auto* transition = new(transitionMem) RHI::RHITransition();
		GRHIApi->RHICreateTransition(transition, RHI::RHITransitionCreateInfo(RHI::ERHITransitionCreateFlags::None, std::move(infos)));

		cmd.BeginTransitions({ transition });
		cmd.EndTransitions({ transition });

		GRHIApi->RHIReleaseTransition(transition);
		delete[] transitionMem;
	}
	RHIUpdateBufferRegion region;
	region.offset = offset;
	region.size = size;

	GRHIApi->UpdateBuffer(cmd, Buffer.get(),data, region);
	cmd.End();

	auto fence = queue->ExecuteContext({ ctx }, {});
	GetTracker().UpdateAccess(ERHIResourceAccess::TransferDest);
	GetTracker().UpdateLastAccessFence(fence);
	queue->WaitFence(fence);
}


RenderTargetPool GRenderTargetPool;


TransientResourceAllocator GTransientResourceAllocator;

// 分配或复用 RenderTarget（重命名GetFreeRenderTarget，增加垃圾回收和分配管理）
std::shared_ptr<PooledRenderTarget> RenderTargetPool::GetFreeRenderTarget(
	const PoolRenderTargetDesc& Desc)
{
	std::lock_guard<std::mutex> Lock(Mutex);
	GarbageCollect();

	if (!FreeList.empty())
	{
		auto it = FreeList.begin();
		std::shared_ptr<PooledRenderTarget> p = nullptr;
		for (it = FreeList.begin(); it != FreeList.end(); ++it) {
			bool match = (*it)->GetDesc().Matches(Desc);
			if (match) {
				break;
			}
		}		
		if (it != FreeList.end()) {
			p = *it;
			FreeList.erase(it);
			AllocatedList.push_back(p);
			return p;
		}
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
			default:
				break;
		}
		if (target && targetFrame < currentFrame && !(target->IsUsed())) {
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
	AllocatedList.clear();
}



RenderTextureSP GlobalTestTexture = nullptr;

bool InitGlobalRenderResource() {
    auto rootPath = Core::GetProjectDir();
	GlobalTestTexture = CreateTexture(rootPath + "/resources/pic/OIP.jpg");
    return true;
}
void ReleaseGlobalRenderResource() {
	GlobalTestTexture.reset();
}

RenderTextureSP CreateTexture(const std::string& Path)
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
	desc.Usage = ERHITextureCreateFlag::ShaderResource | ERHITextureCreateFlag::TransferSrc | ERHITextureCreateFlag::TransferDest;

	// 如果需要生成 Mips，确保 Usage 包含相应的 Flag (如 RenderTarget 或 UAV，取决于 RHI 实现)
	desc.bGenerateMips = false;

	// 如果后续需要组合更多Usage，请通过 ERHITextureCreateFlagsFlags 组合后再 ToEnum() 赋值。


	desc.InitialData = pixels; // 传入 stbi 的内存指针
	desc.DebugName = Path.c_str();

	RenderCore::RenderTextureSP outTexture = nullptr;

	// 3. 调用 RHI 创建纹理
	outTexture = std::make_shared<RenderCore::RenderTexture>(desc);
	outTexture->InitRHIResource();
	auto graphphicContex = GRHIApi->GetQueue(EQueueType::Graphics)->AcquireCastedCommandContext<RHIGraphicContex>();

	RHIGraphicCommandList cmd(graphphicContex);
	cmd.SetImmediate(true);
	cmd.Begin();
	TransitionResource(GRHIApi, cmd, outTexture->GetRHI(), ERHIResourceAccess::Unknown, ERHIResourceAccess::TransferDest,EQueueType::Graphics,EQueueType::Graphics);
	GRHIApi->UpdateTexture(cmd, outTexture->GetRHI(), pixels, RHIUpdateTextureRegion::Create2DRegion(desc.Width, desc.Height));
	cmd.End();
	cmd.ExecuteAll();
	auto fence = GRHIApi->GetQueue(EQueueType::Graphics)->ExecuteContext({ graphphicContex }, {});
	outTexture->GetTracker().UpdateLastAccessFence(fence);
	outTexture->GetTracker().UpdateSubresourceAccess(RHI::RHISubresourceRange{}, ERHIResourceAccess::TransferDest);


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

void TransientResourceAllocator::GarbageCollect() 
{
	auto* api = RHI::GRHIApi;
	if (!api) return;

	uint64_t gfx = api->GetQueue(RHI::EQueueType::Graphics)->GetCurrentTimelineValue();
	uint64_t compute = api->GetQueue(RHI::EQueueType::Compute)->GetCurrentTimelineValue();

	auto IsFenceDone = [&](const RHI::RHIFence& f)
		{
			uint64_t current = 0;

			switch (f.QueueType)
			{
			case RHI::EQueueType::Graphics: current = gfx; break;
			case RHI::EQueueType::Compute:  current = compute; break;
			default: break;
			}

			return f.Value <= current;
		};

	// =========================
	// Texture
	// =========================
	auto itTex = AllocatedTextures.begin();
	while (itTex != AllocatedTextures.end())
	{
		auto& tex = *itTex;

		if (!tex)
		{
			itTex = AllocatedTextures.erase(itTex);
			continue;
		}

		const auto& fence = tex->GetTracker().GetLastAccessFence();

		if (IsFenceDone(fence))
		{
			// 👉 这里才真正释放 RHI 资源
			delete tex->GetRHI();

			itTex = AllocatedTextures.erase(itTex);
		}
		else
		{
			++itTex;
		}
	}

	// =========================
	// Buffer
	// =========================
	auto itBuf = AllocatedBuffers.begin();
	while (itBuf != AllocatedBuffers.end())
	{
		auto& buf = *itBuf;

		if (!buf)
		{
			itBuf = AllocatedBuffers.erase(itBuf);
			continue;
		}

		const auto& fence = buf->GetTracker().GetLastAccessFence();

		if (IsFenceDone(fence))
		{
			delete buf->GetRHI();

			itBuf = AllocatedBuffers.erase(itBuf);
		}
		else
		{
			++itBuf;
		}
	}
}


} // namespace RenderCore