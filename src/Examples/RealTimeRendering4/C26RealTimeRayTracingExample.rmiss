#version 460
#extension GL_EXT_ray_tracing : require

// ���������Ч�غɽṹ��
struct RayPayload {
    vec4 color; // ʹ�� vec4 �洢��ɫ��RGBA��
};
layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    // �������δ�������壬���ر�����ɫ
    payload.color = vec4(0.2, 0.5, 0.8, 1.0); // ǳ��ɫ����
}