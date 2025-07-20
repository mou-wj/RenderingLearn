#include "tiny_gltf.h"
#include "ModelFileTool.h"


void LoadRenderScene(const std::string& modelPath, RenderScene& out) {

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, modelPath.c_str());

    // 纹理
    for (const auto& img : model.images) {
        TextureData tex;
        tex.name = img.name;
        tex.width = img.width;
        tex.height = img.height;
        tex.channels = img.component;
        tex.pixels = img.image;
        tex.uri = img.uri;
        out.textures.push_back(tex);
    }

    // 材质
    for (const auto& mat : model.materials) {
        MaterialData m;
        m.name = mat.name;
        // PBR
        const auto& pbr = mat.pbrMetallicRoughness;
        for (int i = 0; i < 4; ++i) m.baseColorFactor[i] = float(pbr.baseColorFactor[i]);
        m.baseColorTexture = pbr.baseColorTexture.index;
        m.metallicFactor = float(pbr.metallicFactor);
        m.roughnessFactor = float(pbr.roughnessFactor);
        m.metallicRoughnessTexture = pbr.metallicRoughnessTexture.index;
        // 法线贴图
        m.normalTexture = mat.normalTexture.index;
        m.normalScale = float(mat.normalTexture.scale);
        // 遮挡贴图
        m.occlusionTexture = mat.occlusionTexture.index;
        m.occlusionStrength = float(mat.occlusionTexture.strength);
        // 自发光
        m.emissiveTexture = mat.emissiveTexture.index;
        for (int i = 0; i < 3; ++i) m.emissiveFactor[i] = float(mat.emissiveFactor[i]);
        // 透明度
        m.alphaMode = mat.alphaMode;
        m.alphaCutoff = float(mat.alphaCutoff);
        m.doubleSided = mat.doubleSided;
        out.materials.push_back(m);
    }

    // Meshes
    for (const auto& mesh : model.meshes) {
        MeshData meshData;
        meshData.name = mesh.name;
        for (const auto& prim : mesh.primitives) {
            SubMesh sub;
            // 顶点数
            size_t vertexCount = 0;
            if (prim.attributes.count("POSITION")) {
                const auto& accessor = model.accessors[prim.attributes.at("POSITION")];
                vertexCount = accessor.count;
            }
            // 读取各属性
            std::vector<std::vector<float>> attribs(7); // pos, norm, tan, uv, color, joint, weight
            const char* attrNames[7] = { "POSITION", "NORMAL", "TANGENT", "TEXCOORD_0", "COLOR_0", "JOINTS_0", "WEIGHTS_0" };
            int attrComps[7] = { 3, 3, 4, 2, 4, 4, 4 };
            for (int a = 0; a < 7; ++a) {
                if (prim.attributes.count(attrNames[a])) {
                    const auto& acc = model.accessors[prim.attributes.at(attrNames[a])];
                    const auto& view = model.bufferViews[acc.bufferView];
                    const auto& buf = model.buffers[view.buffer];
                    const float* data = reinterpret_cast<const float*>(&buf.data[view.byteOffset + acc.byteOffset]);
                    attribs[a].assign(data, data + acc.count * attrComps[a]);
                }
                else {
                    attribs[a].resize(vertexCount * attrComps[a], (a == 5 || a == 6) ? 0.0f : (a == 4 ? 1.0f : 0.0f));
                }
            }
            // 组装顶点
            for (size_t i = 0; i < vertexCount; ++i) {
                Vertex v;
                for (int j = 0; j < 3; ++j) v.position[j] = attribs[0][i * 3 + j];
                for (int j = 0; j < 3; ++j) v.normal[j] = attribs[1][i * 3 + j];
                for (int j = 0; j < 4; ++j) v.tangent[j] = attribs[2][i * 4 + j];
                for (int j = 0; j < 2; ++j) v.texcoord[j] = attribs[3][i * 2 + j];
                for (int j = 0; j < 4; ++j) v.color[j] = attribs[4][i * 4 + j];
                for (int j = 0; j < 4; ++j) v.joint[j] = attribs[5][i * 4 + j];
                for (int j = 0; j < 4; ++j) v.weight[j] = attribs[6][i * 4 + j];
                sub.vertices.push_back(v);
            }
            // 索引
            if (prim.indices >= 0) {
                const auto& acc = model.accessors[prim.indices];
                const auto& view = model.bufferViews[acc.bufferView];
                const auto& buf = model.buffers[view.buffer];
                const void* data = &buf.data[view.byteOffset + acc.byteOffset];
                for (size_t i = 0; i < acc.count; ++i) {
                    uint32_t idx = 0;
                    switch (acc.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                        idx = ((const uint16_t*)data)[i]; break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                        idx = ((const uint32_t*)data)[i]; break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        idx = ((const uint8_t*)data)[i]; break;
                    }
                    sub.indices.push_back(idx);
                }
            }
            sub.materialIndex = prim.material;
            sub.mode = prim.mode;
            meshData.primitives.push_back(sub);
        }
        out.meshes.push_back(meshData);
    }

    // 节点
    for (const auto& node : model.nodes) {
        NodeData n;
        n.name = node.name;
        n.meshIndex = node.mesh;
        n.children = node.children;
        if (!node.matrix.empty()) {
            for (int i = 0; i < 16; ++i) n.matrix[i] = float(node.matrix[i]);
        }
        if (!node.translation.empty()) {
            for (int i = 0; i < 3; ++i) n.translation[i] = float(node.translation[i]);
        }
        if (!node.rotation.empty()) {
            for (int i = 0; i < 4; ++i) n.rotation[i] = float(node.rotation[i]);
        }
        if (!node.scale.empty()) {
            for (int i = 0; i < 3; ++i) n.scale[i] = float(node.scale[i]);
        }
        out.nodes.push_back(n);
    }

    // 场景
    for (const auto& scene : model.scenes) {
        SceneData s;
        s.name = scene.name;
        s.rootNodes = scene.nodes;
        out.scenes.push_back(s);
    }
    out.defaultScene = model.defaultScene;
}
