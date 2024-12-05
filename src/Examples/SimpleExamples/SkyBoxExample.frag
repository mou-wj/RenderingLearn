#version 450 core


layout(set = 0,binding = 1) uniform samplerCube testTexture;

layout(location = 0) in vec3 inSampleVec;
layout(location = 0) out vec4 outColor;

void main(){

	//由于glsl的采样以坐下角作为采样原点(0,0),故以向量生成采样坐标的时候，texture产生纹理坐标的时候内部会反转生成v的量，所以在一些情况下会导致上下颠倒
	vec3 sampleV = inSampleVec * vec3(1.0,1.0,1.0);
	vec3 texColor = texture(testTexture,sampleV).xyz;
	//gama 解码
	texColor = pow(texColor, vec3(2.4));

	outColor = vec4(texColor,1.0);
}