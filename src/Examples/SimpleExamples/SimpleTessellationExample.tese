#version 450

layout(triangles, equal_spacing, ccw) in;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec3 inColor[];

layout(location = 0) out vec3 fragColor;

void main() {
    // 使用插值系数 gl_TessCoord 计算顶点位置
    vec3 pos = inPosition[0] * gl_TessCoord.x +
               inPosition[1] * gl_TessCoord.y +
               inPosition[2] * gl_TessCoord.z;

   vec3 color = inColor[0] * gl_TessCoord.x +
               inColor[1] * gl_TessCoord.y +
               inColor[2] * gl_TessCoord.z;

    fragColor = color;
    gl_Position = vec4(pos, 1.0);
}