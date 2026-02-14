#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include <future>
#include <iostream>
#include <fstream>
#include <typeindex>
#include <typeinfo>
#include "RenderResource.h"
#include "EngineExport.h"

namespace Engine {



	// ---------------------------
	// Base Asset Interface
	// ---------------------------
	class IAsset
	{
	public:
		virtual ~IAsset() = default;
		virtual const std::string& GetName() const = 0;
		virtual const std::string& GetPath() const = 0;
		virtual bool Load() = 0;
	};


	using AssetSP = std::shared_ptr<IAsset>;



	// ---------------------------
	// Asset Manager
	// ---------------------------
	class ENGINE_API AssetManager
	{
	public:
		using LoadCallback = std::function<void(AssetSP)>;


		static AssetManager& Get();


		// 同步加载
		template<typename AssetType>
		AssetSP LoadSync(const std::string& Path)
		{
			{
				std::lock_guard<std::mutex> lg(Mutex_);
				auto it = Assets_.find(Path);
				if (it != Assets_.end())
					return it->second;
			}


			AssetSP Asset = std::make_shared<AssetType>(Path);
			if (!Asset->Load())
			{
				std::cerr << "Failed to load asset: " << Path << std::endl;
				return nullptr;
			}


			RegisterAsset(Path, Asset);
			return Asset;
		}

		// 异步加载
		template<typename AssetType>
		void LoadAsync(const std::string& Path, LoadCallback Callback)
		{
			{
				std::lock_guard<std::mutex> lg(Mutex_);
				auto it = Assets_.find(Path);
				if (it != Assets_.end())
				{
					Callback(it->second);
					return;
				}
			}


			std::async(std::launch::async, [this, Path, Callback]()
				{
					AssetSP Asset = LoadSync<AssetType>(Path);
					Callback(Asset);
				});
		}


		// 查询已加载
		AssetSP GetAsset(const std::string& Path);


	private:
		AssetManager() = default;
		~AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;


		void RegisterAsset(const std::string& Path, AssetSP Asset);


	private:
		std::unordered_map<std::string, AssetSP> Assets_;
		mutable std::mutex Mutex_;
	};



	class ENGINE_API TextureAsset : public IAsset
	{
	public:
		TextureAsset(const std::string& Path);


		const std::string& GetName() const override;
		const std::string& GetPath() const override;


		// Load 实现
		bool Load() override;

	private:
		std::string Name_ = "TextureAsset";
		std::string Path_;
		size_t Size_ = 0;
		RenderCore::RenderTexture* Texture_ = nullptr;
	};


}
