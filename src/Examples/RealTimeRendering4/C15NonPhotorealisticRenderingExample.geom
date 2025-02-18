#version 450

// ���룺�Ӷ�����ɫ�������Ķ�������
layout(triangles) in;  // ��������Ϊ������

// ��������ݸ�Ƭ����ɫ���Ķ�������
layout(triangle_strip,max_vertices  = 3) out;  // �������Ϊ�����δ���triangle strip�������3������

layout(location = 0) in vec3 inPosition[];//����ռ��е�λ��
layout(location = 1) in vec3 inNormal[];//����ռ��еķ�����
layout(location = 2) in vec3 inTexCoord[];//��������u,v,w
layout(location = 3) in vec3 inTan[];//����ռ��е�Tan����,�ڼ�����ɫ���м���
layout(location = 4) in vec3 inBiTan[];//����ռ��е�BiTan����,�ڼ�����ɫ���м���

layout(location = 0) out vec3 outPosition;//����ռ��е�λ��
layout(location = 1) out vec3 outNormal;//����ռ��еķ�����
layout(location = 2) out vec3 outTexCoord;//��������u,v,w
layout(location = 3) out vec3 outTan;//����ռ��е�Tan����,�ڼ�����ɫ���м���
layout(location = 4) out vec3 outBiTan;//����ռ��е�BiTan����,�ڼ�����ɫ���м���

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
        gl_Position = gl_in[i].gl_Position;  // ����ԭʼ����λ��
        outPosition = inPosition[i];
        outNormal = inNormal[i];
        outTexCoord = inTexCoord[i];
        outTan = tanV;
        outBiTan = biTanV;
        EmitVertex();  // ����һ������

    }
    EndPrimitive();  // �������ͼԪ
}
