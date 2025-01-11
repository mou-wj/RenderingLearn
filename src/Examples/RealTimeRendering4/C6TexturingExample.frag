#version 450 core


layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的位置
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 1) uniform sampler2D pic;



void main(){

	//场景所有对象绘制为白色
	ivec2 size = textureSize(pic,0);
	vec3 sampleColor = texelFetch(pic,ivec2(inTexCoord.xy * size),0).xyz;
	outColor = vec4(sampleColor,1);

}