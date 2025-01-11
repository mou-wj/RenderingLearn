#version 450 core


layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��е�λ��
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 1) uniform sampler2D pic;



void main(){

	//�������ж������Ϊ��ɫ
	ivec2 size = textureSize(pic,0);
	vec3 sampleColor = texelFetch(pic,ivec2(inTexCoord.xy * size),0).xyz;
	outColor = vec4(sampleColor,1);

}