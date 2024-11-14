#version 450 core


layout(set = 0,binding = 1) uniform samplerCube testTexture;

layout(location = 0) in vec3 inSampleVec;
layout(location = 0) out vec4 outColor;

void main(){

	vec4 texColor = texture(testTexture,inSampleVec);
	outColor = texColor;

}