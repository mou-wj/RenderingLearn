#version 460
#extension GL_EXT_ray_tracing : require

// 定义光线有效载荷结构体
struct RayPayload {
    vec4 color; // 使用 vec4 存储颜色（RGBA）
};
layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    // 如果光线未命中物体，返回背景颜色
    payload.color = vec4(0.2, 0.5, 0.8, 1.0); // 浅蓝色背景
}