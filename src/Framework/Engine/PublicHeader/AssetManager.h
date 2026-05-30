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
#include "Material.h"

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

    struct AssetInfo
    {
        std::string Name;
        std::string Path;
        std::type_index Type;
        AssetInfo(): Name(""), Path(), Type(typeid(IAsset)){}
		AssetInfo(const std::string& name,const std::string& path,const std::type_index& type): Name(name), Path(path), Type(type){}
        AssetInfo(const AssetInfo& other) : AssetInfo(other.Name,other.Path,other.Type){}
        // 未来可以扩展
        // std::vector<AssetID> Dependencies;
    };

    class ENGINE_API AssetRegistry
    {
    public:
        static AssetRegistry& Get()
        {
            static AssetRegistry Instance;
            return Instance;
        }

    public:

        void Register(const AssetInfo& Info)
        {
            std::lock_guard<std::mutex> lock(Mutex);
			PathToInfo.emplace(Info.Path, Info);
            //PathToInfo[Info.Path] = Info;
            NameToPaths[Info.Name].push_back(Info.Path);
        }

        const AssetInfo* GetByPath(const std::string& Path)
        {
            auto it = PathToInfo.find(Path);
            if (it == PathToInfo.end()) return nullptr;
            return &it->second;
        }

        std::vector<const AssetInfo*> GetByName(const std::string& Name)
        {
            std::vector<const AssetInfo*> Result;

            auto it = NameToPaths.find(Name);
            if (it == NameToPaths.end()) return Result;

            for (auto& path : it->second)
            {
                Result.push_back(&PathToInfo[path]);
            }

            return Result;
        }

    private:
        std::unordered_map<std::string, AssetInfo> PathToInfo;
        std::unordered_map<std::string, std::vector<std::string>> NameToPaths;

        std::mutex Mutex;
    };

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
            {
                std::lock_guard<std::mutex> lock(Mutex);

                auto it = PathToAsset.find(Path);
                if (it != PathToAsset.end())
                {
                    return std::static_pointer_cast<AssetType>(it->second);
                }
            }

            auto asset = std::make_shared<AssetType>(Path);

            if (!asset->Load())
            {
                return nullptr;
            }

            //
            AssetInfo info(Path, asset->GetName(), std::type_index(typeid(AssetType)));

            AssetRegistry::Get().Register(info);

            {
                std::lock_guard<std::mutex> lock(Mutex);
                PathToAsset[Path] = asset;
            }

            return asset;
        }


        template<typename AssetType>
        void LoadAsync(
            const std::string& Path,
            std::function<void(std::shared_ptr<AssetType>)> Callback)
        {
            // -----------------------------
            // 1. Cache 命中直接返回
            // -----------------------------
            {
                std::lock_guard<std::mutex> lock(Mutex);

                auto it = PathToAsset.find(Path);
                if (it != PathToAsset.end())
                {
                    Callback(std::static_pointer_cast<AssetType>(it->second));
                    return;
                }

                // -----------------------------
                // 2. 防止重复加载
                // -----------------------------
                if (LoadingSet.contains(Path))
                {
                    return; // 或者可以挂等待队列
                }

                LoadingSet.insert(Path);
            }

            // -----------------------------
            // 3. 异步加载
            // -----------------------------
            std::async(std::launch::async,
                [this, Path, Callback]()
                {
                    // 内部调用 Sync Loader
                    auto asset = LoadSync<AssetType>(Path);

                    {
                        std::lock_guard<std::mutex> lock(Mutex);
                        LoadingSet.erase(Path);
                    }

                    Callback(asset);
                });
        }

        /*
        -------------------------------------------------------------------------------
            Get Loaded Asset
        -------------------------------------------------------------------------------
        */
        template<typename AssetType>
        const AssetType* GetAsset(const std::string& Path)
        {
            std::lock_guard<std::mutex> lock(Mutex);

            auto it = PathToAsset.find(Path);
            if (it == PathToAsset.end())
                return nullptr;

            return dynamic_cast<AssetType*>(it->second.get());
        }

        template<typename AssetType>
        const AssetType* GetByName(const std::string& Name)
        {
            auto infos = AssetRegistry::Get().GetByName(Name);

            if (infos.empty())
                return nullptr;

            // 默认取第一个（也可以返回数组）
            const auto* info = infos[0];

            return GetAsset<AssetType>(info->Path);
        }

        /*
        -------------------------------------------------------------------------------
            Clear Cache
        -------------------------------------------------------------------------------
        */
        void Clear()
        {
            std::lock_guard<std::mutex> lock(Mutex);
            PathToAsset.clear();
        }

    private:
        AssetManager() = default;
        ~AssetManager() = default;

        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;

    private:

        // type -> path -> asset
        std::unordered_map<std::string, AssetSP> PathToAsset;
        std::unordered_set<std::string> LoadingSet;
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
