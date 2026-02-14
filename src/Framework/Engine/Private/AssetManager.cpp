#include "AssetManager.h"
using namespace RHI;
using namespace RenderCore;
namespace Engine {
	// ---------------------------
	// µ¥Àý
	// ---------------------------
	AssetManager& AssetManager::Get()
	{
		static AssetManager Instance;
		return Instance;
	}


	void AssetManager::RegisterAsset(const std::string& Path, AssetSP Asset)
	{
		std::lock_guard<std::mutex> lg(Mutex_);
		Assets_[Path] = Asset;
	}


	AssetSP AssetManager::GetAsset(const std::string& Path)
	{
		std::lock_guard<std::mutex> lg(Mutex_);
		auto it = Assets_.find(Path);
		return (it != Assets_.end()) ? it->second : nullptr;
	}


	TextureAsset::TextureAsset(const std::string& Path) : Path_(Path)
	{
		Texture_ = new RenderCore::RenderTexture();
	}


	const std::string& TextureAsset::GetName() const
	{
		return Name_;
	}


	const std::string& TextureAsset::GetPath() const
	{
		return Path_;
	}


	bool TextureAsset::Load()
	{
		Texture_ = CreateTexture(Path_);
		return Texture_ != nullptr;
	}

	

}