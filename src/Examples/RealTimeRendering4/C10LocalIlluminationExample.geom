#version 450

// 输入：从顶点着色器传来的顶点数据
layout(triangles) in;  // 输入类型为三角形

// 输出：传递给片段着色器的顶点数据
layout(triangle_strip,max_vertices  = 3) out;  // 输出类型为三角形带（triangle strip），最多3个顶点

layout(location = 0) in vec3 inPosition[];//世界空间中的位置
layout(location = 1) in vec3 inNormal[];//世界空间中的法向量
layout(location = 2) in vec3 inTexCoord[];//纹理坐标u,v,w
layout(location = 3) in vec3 inTan[];//世界空间中的Tan向量,在几何着色器中计算
layout(location = 4) in vec3 inBiTan[];//世界空间中的BiTan向量,在几何着色器中计算

layout(location = 0) out vec3 outPosition;//世界空间中的位置
layout(location = 1) out vec3 outNormal;//世界空间中的法向量
layout(location = 2) out vec3 outTexCoord;//纹理坐标u,v,w
layout(location = 3) out vec3 outTan;//世界空间中的Tan向量,在几何着色器中计算
layout(location = 4) out vec3 outBiTan;//世界空间中的BiTan向量,在几何着色器中计算

vec3 calculateTangent(vec3 P0, vec3 P1, vec3 P2, vec2 UV0, vec2 UV1, vec2 UV2) {
    vec3 edge1 = P1 - P0;
    vec3 edge2 = P2 - P0;
    vec2 deltaUV1 = UV1 - UV0;
    vec2 deltaUV2 = UV2 - UV0;

    float f = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

    vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
    return normalize(tangent);
}

vec3 calculateBitangent(vec3 P0, vec3 P1, vec3 P2, vec2 UV0, vec2 UV1, vec2 UV2) {
    vec3 edge1 = P1 - P0;
    vec3 edge2 = P2 - P0;
    vec2 deltaUV1 = UV1 - UV0;
    vec2 deltaUV2 = UV2 - UV0;

    float f = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

    vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
    return normalize(bitangent);
}

void main() {
    vec3 tanV = calculateTangent(inPosition[0],inPosition[1],inPosition[2],inTexCoord[0].xy,inTexCoord[1].xy,inTexCoord[2].xy);
    vec3 biTanV = calculateBitangent(inPosition[0],inPosition[1],inPosition[2],inTexCoord[0].xy,inTexCoord[1].xy,inTexCoord[2].xy);



    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;  // 传递原始顶点位置
        outPosition = inPosition[i];
        outNormal = inNormal[i];
        outTexCoord = inTexCoord[i];
        outTan = tanV;
        outBiTan = biTanV;
        EmitVertex();  // 发射一个顶点

    }
    EndPrimitive();  // 结束这个图元
}
