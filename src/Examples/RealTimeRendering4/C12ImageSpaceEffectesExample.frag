#version 450 core

layout(location = 0) in vec2 inUV;//屏幕空间中对应的UV值

layout(set = 0,binding = 0) uniform sampler2D postProcessTexture;

layout(location = 0) out vec4 outColor;

void main(){
	outColor = texture(postProcessTexture,inUV).xyzw;
}