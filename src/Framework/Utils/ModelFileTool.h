#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>
#include "glm/glm.hpp"

// 顶点属性
struct Vertex {
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec4 tangent{};
    glm::vec2 texcoord{};
    glm::vec4 color{};
    glm::vec4 joint{};
    glm::vec4 weight{};
};

// 索引
using Indices = std::vector<uint32_t>;

// 纹理
struct TextureData {
    std::string name;
    int width = 0, height = 0, channels = 0;
    std::vector<unsigned char> pixels;
    std::string uri;
};

// 材质
struct MaterialData {
    std::string name;
    std::array<float, 4> baseColorFactor{ 1,1,1,1 };
    int baseColorTexture = -1;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    int metallicRoughnessTexture = -1;
    int normalTexture = -1;
    float normalScale = 1.0f;
    int occlusionTexture = -1;
    float occlusionStrength = 1.0f;
    int emissiveTexture = -1;
    std::array<float, 3> emissiveFactor{ 0,0,0 };
    std::string alphaMode = "OPAQUE";
    float alphaCutoff = 0.5f;
    bool doubleSided = false;
};

// 子网格/Primitive
struct SubMesh {
    std::vector<Vertex> vertices;
    Indices indices;
    int materialIndex = -1;
    int mode = 4; // triangles
};

// Mesh
struct MeshData {
    std::string name;
    std::vector<SubMesh> primitives;
};

// 节点
struct NodeData {
    std::string name;
    int meshIndex = -1;
    int parent = -1;
    std::vector<int> children;
    std::array<float, 16> matrix{}; // 4x4
    std::array<float, 3> translation{};
    std::array<float, 4> rotation{};
    std::array<float, 3> scale{ 1,1,1 };
};

// 场景
struct SceneData {
    std::string name;
    std::vector<int> rootNodes;
};

// 总体
struct RenderScene {
    std::vector<MeshData> meshes;
    std::vector<MaterialData> materials;
    std::vector<TextureData> textures;
    std::vector<NodeData> nodes;
    std::vector<SceneData> scenes;
    int defaultScene = 0;
};

void LoadRenderScene(const std::string& modelPath, RenderScene& out);