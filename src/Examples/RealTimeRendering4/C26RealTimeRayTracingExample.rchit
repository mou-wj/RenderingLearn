#version 460
#extension GL_EXT_ray_tracing : require

// ���������Ч�غɽṹ��
struct RayPayload {
    vec4 color; // ʹ�� vec4 �洢��ɫ��RGBA��
};
layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    // ��������������壬���غ�ɫ
    payload.color = vec4(1.0, 0.0, 0.0, 1.0); // ��ɫ����ȫ��͸��
}