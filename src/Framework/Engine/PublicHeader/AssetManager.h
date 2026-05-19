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
#include "StaticMesh.h"
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


   /*
    ===============================================================================
        Asset Manager
    ===============================================================================
    */
    class ENGINE_API AssetManager
    {
    public:
        using LoadCallback = std::function<void(AssetSP)>;

    public:
        static AssetManager& Get()
        {
            static AssetManager Instance;
            return Instance;
        }

    public:

        /*
        -------------------------------------------------------------------------------
            Sync Load
        -------------------------------------------------------------------------------
        */
        template<typename AssetType>
        std::shared_ptr<AssetType> LoadSync(const std::string& Path)
        {
            const std::type_index type = std::type_index(typeid(AssetType));

            {
                std::lock_guard<std::mutex> lock(Mutex);

                auto typeIt = Assets.find(type);
                if (typeIt != Assets.end())
                {
                    auto assetIt = typeIt->second.find(Path);
                    if (assetIt != typeIt->second.end())
                    {
                        return std::static_pointer_cast<AssetType>(
                            assetIt->second);
                    }
                }
            }

            auto asset = std::make_shared<AssetType>(Path);

            if (!asset->Load())
            {
                std::cerr
                    << "Failed to load asset: "
                    << Path
                    << std::endl;

                return nullptr;
            }

            {
                std::lock_guard<std::mutex> lock(Mutex);

                Assets[type][Path] = asset;
            }

            return asset;
        }

        /*
        -------------------------------------------------------------------------------
            Async Load
        -------------------------------------------------------------------------------
        */
        template<typename AssetType>
        void LoadAsync(
            const std::string& Path,
            std::function<void(std::shared_ptr<AssetType>)> Callback)
        {
            {
                std::lock_guard<std::mutex> lock(Mutex);

                const std::type_index type =
                    std::type_index(typeid(AssetType));

                auto typeIt = Assets.find(type);

                if (typeIt != Assets.end())
                {
                    auto assetIt = typeIt->second.find(Path);

                    if (assetIt != typeIt->second.end())
                    {
                        Callback(
                            std::static_pointer_cast<AssetType>(
                                assetIt->second));

                        return;
                    }
                }
            }

            std::async(
                std::launch::async,
                [this, Path, Callback]()
                {
                    auto asset =
                        LoadSync<AssetType>(Path);

                    Callback(asset);
                });
        }

        /*
        -------------------------------------------------------------------------------
            Get Loaded Asset
        -------------------------------------------------------------------------------
        */
        template<typename AssetType>
        std::shared_ptr<AssetType> GetAsset(
            const std::string& Path)
        {
            std::lock_guard<std::mutex> lock(Mutex);

            const std::type_index type =
                std::type_index(typeid(AssetType));

            auto typeIt = Assets.find(type);
            if (typeIt == Assets.end())
            {
                return nullptr;
            }

            auto assetIt =
                typeIt->second.find(Path);

            if (assetIt == typeIt->second.end())
            {
                return nullptr;
            }

            return std::static_pointer_cast<AssetType>(
                assetIt->second);
        }

        /*
        -------------------------------------------------------------------------------
            Clear Cache
        -------------------------------------------------------------------------------
        */
        void Clear()
        {
            std::lock_guard<std::mutex> lock(Mutex);
            Assets.clear();
        }

    private:
        AssetManager() = default;
        ~AssetManager() = default;

        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;

    private:

        // type -> path -> asset
        std::unordered_map<
            std::type_index,
            std::unordered_map<std::string, AssetSP>
        > Assets;

        mutable std::mutex Mutex;
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
		std::shared_ptr <RenderCore::RenderTexture> Texture_ = nullptr;
	};

	class ENGINE_API StaticMeshAsset : public IAsset
	{
	public:
		StaticMeshAsset(const std::string& Path);


		const std::string& GetName() const override;
		const std::string& GetPath() const override;


		// Load 实现
		bool Load() override;

        StaticMesh* GetMesh() const { return Mesh_.get(); }

	private:
		std::string Name_ = "StaticMeshAsset";
		std::string Path_;
		size_t Size_ = 0;
		std::shared_ptr <StaticMesh> Mesh_ = nullptr;
	};

	/*
===============================================================================
	Material Asset
===============================================================================
*/
	class ENGINE_API MaterialAsset : public IAsset
	{
	public:
		explicit MaterialAsset(const std::string& InPath)
			: Path(InPath)
		{
		}

		~MaterialAsset() override = default;

		const std::string& GetName() const override
		{
			return Name;
		}

		const std::string& GetPath() const override
		{
			return Path;
		}

		bool Load() override;

		MaterialInterface* GetMaterial() const
		{
			return MaterialPtr.get();
		}

	private:
		std::string Name = "MaterialAsset";
		std::string Path;

		std::shared_ptr<Material> MaterialPtr;
	};
}
