#include "AssetManager.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_gltf.h"

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
        tinygltf::TinyGLTF Loader;
        tinygltf::Model Model;

        std::string Error;
        std::string Warning;

        bool Result = false;

        if (Path.ends_with(".glb"))
        {
            Result = Loader.LoadBinaryFromFile(
                &Model,
                &Error,
                &Warning,
                Path);
        }
        else
        {
            Result = Loader.LoadASCIIFromFile(
                &Model,
                &Error,
                &Warning,
                Path);
        }

        if (!Warning.empty())
        {
            std::cout
                << "[tinygltf warning] "
                << Warning
                << std::endl;
        }

        if (!Error.empty())
        {
            std::cerr
                << "[tinygltf error] "
                << Error
                << std::endl;
        }

        if (!Result)
        {
            return nullptr;
        }

        auto RenderData =
            std::make_shared<StaticMeshRenderData>();

        LODResource LOD0;

        for (const auto& Mesh : Model.meshes)
        {
            for (const auto& Primitive : Mesh.primitives)
            {
                SectionInfo Section;

                uint32_t BaseVertexIndex =
                    static_cast<uint32_t>(
                        LOD0.VertexBuffers
                        .PositionBuffer
                        .GetNumVertices());

                //-------------------------------------
                // POSITION
                //-------------------------------------

                auto PositionIt =
                    Primitive.attributes.find("POSITION");

                if (PositionIt == Primitive.attributes.end())
                {
                    continue;
                }

                ReadVec3Attribute(
                    Model,
                    PositionIt->second,
                    LOD0.VertexBuffers.PositionBuffer);

                //-------------------------------------
                // NORMAL
                //-------------------------------------

                auto NormalIt =
                    Primitive.attributes.find("NORMAL");

                if (NormalIt != Primitive.attributes.end())
                {
                    ReadVec3Attribute(
                        Model,
                        NormalIt->second,
                        LOD0.VertexBuffers.NormalBuffer);
                }

                //-------------------------------------
                // UV0
                //-------------------------------------

                auto UVIt =
                    Primitive.attributes.find("TEXCOORD_0");

                if (UVIt != Primitive.attributes.end())
                {
                    ReadVec2Attribute(
                        Model,
                        UVIt->second,
                        LOD0.VertexBuffers.UVBuffer);
                }

                //-------------------------------------
                // Indices
                //-------------------------------------

                if (Primitive.indices >= 0)
                {
                    const auto& Accessor =
                        Model.accessors[Primitive.indices];

                    Section.FirstIndex =
                        static_cast<uint32_t>(
                            LOD0.IndexBuffer.Indices.size());

                    Section.NumIndices =
                        static_cast<uint32_t>(
                            Accessor.count);

                    Section.MaterialIndex =
                        Primitive.material;

                    Section.BaseVertexIndex =
                        static_cast<int32_t>(
                            BaseVertexIndex);

                    if (Accessor.componentType ==
                        TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        const uint16_t* Indices =
                            GetBufferData<uint16_t>(
                                Model,
                                Accessor);

                        for (size_t i = 0;
                            i < Accessor.count;
                            ++i)
                        {
                            LOD0.IndexBuffer.Indices.push_back(
                                static_cast<uint32_t>(
                                    Indices[i])
                                + BaseVertexIndex);
                        }
                    }
                    else if (
                        Accessor.componentType ==
                        TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        const uint32_t* Indices =
                            GetBufferData<uint32_t>(
                                Model,
                                Accessor);

                        for (size_t i = 0;
                            i < Accessor.count;
                            ++i)
                        {
                            LOD0.IndexBuffer.Indices.push_back(
                                Indices[i]
                                + BaseVertexIndex);
                        }
                    }

                    LOD0.Sections.push_back(Section);
                }
            }
        }

        //-------------------------------------
        // Bounds
        //-------------------------------------

        const auto& Positions =
            LOD0.VertexBuffers.PositionBuffer.Vertices;

        if (!Positions.empty())
        {
            Core::Float3 Min(
                FLT_MAX,
                FLT_MAX,
                FLT_MAX);

            Core::Float3 Max(
                -FLT_MAX,
                -FLT_MAX,
                -FLT_MAX);

            for (size_t i = 0; i < Positions.size(); i += 3)
            {
                Core::Float3 P(
                    Positions[i + 0],
                    Positions[i + 1],
                    Positions[i + 2]);

                Min = CORE_MIN(Min, P);
                Max = CORE_MAX(Max, P);
            }

            RenderData->Bounds =
                Core::AABB(Min, Max);
        }

        RenderData->AddLOD(std::move(LOD0));

        return std::make_shared<StaticMesh>(RenderData);
    }
	bool StaticMeshAsset::Load()
	{
		Mesh_ = LoadMesh(Path_);
		return Mesh_ != nullptr;
	}

    bool MaterialAsset::Load() { return true; }
}