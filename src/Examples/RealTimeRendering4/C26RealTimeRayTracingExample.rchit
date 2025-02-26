#version 460
#extension GL_EXT_ray_tracing : require

// 定义光线有效载荷结构体
struct RayPayload {
    vec4 color; // 使用 vec4 存储颜色（RGBA）
};
layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    // 如果光线命中物体，返回红色
    payload.color = vec4(1.0, 0.0, 0.0, 1.0); // 红色，完全不透明
}