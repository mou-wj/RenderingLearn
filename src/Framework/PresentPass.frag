#version 450 core

layout(set = 0,binding = 1) uniform sampler2D testTexture;
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main(){

		vec4 texColor = texture(testTexture,inUV);
		outColor = texColor;
		
}