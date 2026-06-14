#include "AssetManager.h"
#include "EngineGlobal.h"
#include "RenderInterface.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_gltf.h"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace RHI;
using namespace RenderCore;
namespace Engine {



	TextureAsset::TextureAsset(const std::string& Path) : Path_(Path)
	{
		
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
        Name_ = Core::GetFileName(Path_);
		return Texture_ != nullptr;
	}


	StaticMeshAsset::StaticMeshAsset(const std::string& Path) : Path_(Path)
	{

	}


	const std::string& StaticMeshAsset::GetName() const
	{
		return Name_;
	}


	const std::string& StaticMeshAsset::GetPath() const
	{
		return Path_;
	}

    namespace
    {
        template<typename T>
        const T* GetBufferData(
            const tinygltf::Model& Model,
            const tinygltf::Accessor& Accessor)
        {
            const auto& BufferView =
                Model.bufferViews[Accessor.bufferView];

            const auto& Buffer =
                Model.buffers[BufferView.buffer];

            const uint8_t* Data =
                Buffer.data.data()
                + BufferView.byteOffset
                + Accessor.byteOffset;

            return reinterpret_cast<const T*>(Data);
        }

        void ReadVec3Attribute(
            const tinygltf::Model& Model,
            int AccessorIndex,
            VertexBuffer& OutBuffer)
        {
            if (AccessorIndex < 0)
            {
                return;
            }

            const auto& Accessor =
                Model.accessors[AccessorIndex];

            const float* Data =
                GetBufferData<float>(Model, Accessor);

            OutBuffer.NumComponents = 3;
            OutBuffer.Valid = true;

            OutBuffer.Vertices.reserve(
                OutBuffer.Vertices.size()
                + Accessor.count * 3);

            for (size_t i = 0; i < Accessor.count; ++i)
            {
                OutBuffer.Vertices.push_back(Data[i * 3 + 0]);
                OutBuffer.Vertices.push_back(Data[i * 3 + 1]);
                OutBuffer.Vertices.push_back(Data[i * 3 + 2]);
            }
        }

        void ReadVec2Attribute(
            const tinygltf::Model& Model,
            int AccessorIndex,
            VertexBuffer& OutBuffer)
        {
            if (AccessorIndex < 0)
            {
                return;
            }

            const auto& Accessor =
                Model.accessors[AccessorIndex];

            const float* Data =
                GetBufferData<float>(Model, Accessor);

            OutBuffer.NumComponents = 2;
            OutBuffer.Valid = true;

            OutBuffer.Vertices.reserve(
                OutBuffer.Vertices.size()
                + Accessor.count * 2);

            for (size_t i = 0; i < Accessor.count; ++i)
            {
                OutBuffer.Vertices.push_back(Data[i * 2 + 0]);
                OutBuffer.Vertices.push_back(Data[i * 2 + 1]);
            }
        }
    }

    StaticMeshSP LoadMesh(const std::string& Path)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        bool ret = false;
        if (Path.ends_with(".glb")) {
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, Path.c_str());
        }
        else {
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, Path.c_str());
        }

        if (!warn.empty())
            std::cout << "GLTF Warning: " << warn << std::endl;
        if (!err.empty())
            std::cerr << "GLTF Error: " << err << std::endl;
        if (!ret) return nullptr;

        auto RenderData = std::make_shared<StaticMeshRenderData>();
        auto staticMesh = std::make_shared<StaticMesh>(RenderData);

        // ------------------- ���� -------------------
        std::vector<MaterialInterface*> Materials(model.materials.size());

        for (size_t i = 0; i < model.materials.size(); ++i) {
            
            const auto& mat = model.materials[i];
            auto M = new Material();
            M->SetBlendMode(EBlendMode::Opaque);
            M->SetShadingModel(EShadingModel::Lit);

            // BaseColor
            if (!mat.pbrMetallicRoughness.baseColorFactor.empty()) {
                std::array<float, 4> color = { 1,1,1,1 };
                for (int c = 0; c < 4; ++c)
                    color[c] = float(mat.pbrMetallicRoughness.baseColorFactor[c]);
                M->SetParameterValue<EMaterialParameterSemantic::BaseColor, FVectorType>("BaseColor", color);
            }

            // Metallic / Roughness
            M->SetParameterValue<EMaterialParameterSemantic::Metallic, FScalarType>("Metallic", float(mat.pbrMetallicRoughness.metallicFactor));
            M->SetParameterValue<EMaterialParameterSemantic::Roughness, FScalarType>("Roughness", float(mat.pbrMetallicRoughness.roughnessFactor));

            // Alpha mode
            if (mat.alphaMode == "OPAQUE") M->SetBlendMode(EBlendMode::Opaque);
            else if (mat.alphaMode == "MASK") M->SetBlendMode(EBlendMode::Masked);
            else if (mat.alphaMode == "BLEND") M->SetBlendMode(EBlendMode::Translucent);
            Materials[i] = M;
        }
        bool useDefalutMaterial = false;
        if (Materials.empty()) {
            useDefalutMaterial = true;
			auto defaultMaterialAsset = AssetManager::Get().GetAsset<MaterialAsset>(Core::GetProjectDir() + "/resources/material/DefaultWhite/material.json");
            auto mMaterialAsset = defaultMaterialAsset;
			auto materialInstance = new MaterialInstance(mMaterialAsset->GetMaterial());
            Materials.push_back(materialInstance);
        }


        // ------------------- Mesh LODs -------------------
        for (const auto& mesh : model.meshes) {
            LODResource LOD;
            Core::BoxSphereBounds& Bounds = RenderData->Bounds;
            for (const auto& prim : mesh.primitives) {
                // ����
                size_t vertexCount = 0;
                
                if (prim.attributes.count("POSITION")) {
                    const auto& accessor = model.accessors[prim.attributes.at("POSITION")];
                    vertexCount = accessor.count;

                    const auto& view = model.bufferViews[accessor.bufferView];
                    const auto& buf = model.buffers[view.buffer];
                    const float* data = reinterpret_cast<const float*>(&buf.data[view.byteOffset + accessor.byteOffset]);
                    LOD.VertexBuffers.PositionBuffer.Vertices.assign(data, data + vertexCount * 3);
                    LOD.VertexBuffers.PositionBuffer.NumComponents = 3;
                    LOD.VertexBuffers.PositionBuffer.Valid = true;
                    Core::Float3 v = Core::Float3(data[0], data[1], data[2]);
                    Bounds.Box.Min = CORE_MIN(v, Bounds.Box.Min);
                    Bounds.Box.Max = CORE_MAX(v, Bounds.Box.Max);

                }

                // Normal
                if (prim.attributes.count("NORMAL")) {
                    const auto& accessor = model.accessors[prim.attributes.at("NORMAL")];
                    const auto& view = model.bufferViews[accessor.bufferView];
                    const auto& buf = model.buffers[view.buffer];
                    const float* data = reinterpret_cast<const float*>(&buf.data[view.byteOffset + accessor.byteOffset]);
                    LOD.VertexBuffers.NormalBuffer.Vertices.assign(data, data + vertexCount * 3);
                    LOD.VertexBuffers.NormalBuffer.NumComponents = 3;
                    LOD.VertexBuffers.NormalBuffer.Valid = true;
                }

                // UV
                if (prim.attributes.count("TEXCOORD_0")) {
                    const auto& accessor = model.accessors[prim.attributes.at("TEXCOORD_0")];
                    const auto& view = model.bufferViews[accessor.bufferView];
                    const auto& buf = model.buffers[view.buffer];
                    const float* data = reinterpret_cast<const float*>(&buf.data[view.byteOffset + accessor.byteOffset]);
                    LOD.VertexBuffers.UVBuffer.Vertices.assign(data, data + vertexCount * 2);
                    LOD.VertexBuffers.UVBuffer.NumComponents = 2;
                    LOD.VertexBuffers.UVBuffer.Valid = true;
                }

                // ����
                if (prim.indices >= 0) {
                    const auto& accessor = model.accessors[prim.indices];
                    const auto& view = model.bufferViews[accessor.bufferView];
                    const auto& buf = model.buffers[view.buffer];
                    const void* ptr = &buf.data[view.byteOffset + accessor.byteOffset];
                    for (size_t i = 0; i < accessor.count; ++i) {
                        uint32_t idx = 0;
                        switch (accessor.componentType) {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: idx = ((const uint16_t*)ptr)[i]; break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: idx = ((const uint32_t*)ptr)[i]; break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: idx = ((const uint8_t*)ptr)[i]; break;
                        }
                        LOD.IndexBuffer.Indices.push_back(idx);
                    }
                }

                // Section
                SectionInfo section;
                section.FirstIndex = 0; // TODO: �ۼ�
                section.NumIndices = LOD.IndexBuffer.Indices.size();
                section.MaterialIndex = prim.material;
                if(useDefalutMaterial) section.MaterialIndex = 0;
                LOD.Sections.push_back(section);
            }
            LOD.InitializeResources();
            RenderData->AddLOD(std::move(LOD));
        }
        for (auto materil : Materials) {
			staticMesh->AddMaterial(materil);
        }
        return staticMesh;
    }
	bool StaticMeshAsset::Load()
	{
		Mesh_ = LoadMesh(Path_);
        Name_ = Core::GetFileName(Path_);
		return Mesh_ != nullptr;
	}



    using json = nlohmann::json;
    bool MaterialAsset::Load() { 
        std::ifstream file(Path);

        if (!file.is_open())
        {
            return false;
        }

        json materialJson;

        try
        {
            file >> materialJson;
        }
        catch (...)
        {
            return false;
        }

        auto NewMaterial =
            std::make_shared<Material>();
        if (materialJson.contains("name"))
        {
            const std::string materialName =
                materialJson["name"]
                .get<std::string>();
			Name = materialName;
        }

        /*
        ===========================================================================
            Blend Mode
        ===========================================================================
        */

        if (materialJson.contains("blendMode"))
        {
            const std::string BlendMode =
                materialJson["blendMode"]
                .get<std::string>();

            if (BlendMode == "Opaque")
            {
                NewMaterial->SetBlendMode(
                    EBlendMode::Opaque);
            }
            else if (BlendMode == "Masked")
            {
                NewMaterial->SetBlendMode(
                    EBlendMode::Masked);
            }
            else if (BlendMode == "Translucent")
            {
                NewMaterial->SetBlendMode(
                    EBlendMode::Translucent);
            }
        }

        /*
        ===========================================================================
            Shading Model
        ===========================================================================
        */

        if (materialJson.contains("shadingModel"))
        {
            const std::string ShadingModel =
                materialJson["shadingModel"]
                .get<std::string>();

            if (ShadingModel == "Lit")
            {
                NewMaterial->SetShadingModel(
                    EShadingModel::Lit);
            }
            else if (ShadingModel == "Unlit")
            {
                NewMaterial->SetShadingModel(
                    EShadingModel::Unlit);
            }
        }

        /*
        ===========================================================================
            Parameters
        ===========================================================================
        */

        if (materialJson.contains("parameters"))
        {
            const auto& Params =
                materialJson["parameters"];

            /*
            -----------------------------------------------------------------------
                BaseColor
            -----------------------------------------------------------------------
            */

            if (Params.contains("BaseColor"))
            {
                const auto& Value =
                    Params["BaseColor"];

                std::array<float, 4>
                    BaseColor =
                {
                    Value[0].get<float>(),
                    Value[1].get<float>(),
                    Value[2].get<float>(),
                    Value[3].get<float>()
                };

                NewMaterial
                    ->SetParameterValue<
                    EMaterialParameterSemantic
                    ::BaseColor,
                    FVectorType>(
                        "BaseColor",
                        BaseColor);
            }

            /*
            -----------------------------------------------------------------------
                Roughness
            -----------------------------------------------------------------------
            */

            if (Params.contains("Roughness"))
            {
                const float Roughness =
                    Params["Roughness"]
                    .get<float>();

                NewMaterial
                    ->SetParameterValue<
                    EMaterialParameterSemantic
                    ::Roughness,
                    FScalarType>(
                        "Roughness",
                        Roughness);
            }

            /*
            -----------------------------------------------------------------------
                Metallic
            -----------------------------------------------------------------------
            */

            if (Params.contains("Metallic"))
            {
                const float Metallic =
                    Params["Metallic"]
                    .get<float>();

                NewMaterial
                    ->SetParameterValue<
                    EMaterialParameterSemantic
                    ::Metallic,
                    FScalarType>(
                        "Metallic",
                        Metallic);
            }
        }

        MaterialPtr =
            std::move(NewMaterial);

        return true;
    }


    bool SkyLightAsset::Load(){
		uint32_t cubeWidth = 512;
        uint32_t cubeHeight = 512;
        uint32_t roughnessCount = 8;
        HDRTexture = CreateTexture(Path);
        Name = Core::GetFileName(Path);
        RHI::RHITextureDesc Desc;
        Desc.Format = RHI::ERHIFormat::B8G8R8A8_UNorm;
        Desc.Width = cubeWidth;
        Desc.Height = cubeHeight;
		Desc.ArraySize = 6;
        Desc.MipLevels = 1;
        Desc.Usage = RHI::ERHITextureCreateFlag::ShaderResource | RHI::ERHITextureCreateFlag::UAV;
		Desc.Type = RHI::ERHITextureType::TextureCube;
        DiffuseIrradiance = std::make_shared<RenderCore::RenderTexture>(Desc);
        DiffuseIrradiance->InitRHIResource();
        Desc.MipLevels = 6;
        SpecularPrefilter = std::make_shared<RenderCore::RenderTexture>(Desc);
        SpecularPrefilter->InitRHIResource();
        GetRenderModuleInstance()->PreComputeIBL(HDRTexture.get(), DiffuseIrradiance.get(), SpecularPrefilter.get());
        return true;
    }
}