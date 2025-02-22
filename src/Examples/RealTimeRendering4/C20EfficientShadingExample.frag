#version 450 core


layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor;

layout(input_attachment_index = 0,set = 0,binding = 1) uniform subpassInput GBuffer1;
layout(input_attachment_index = 1,set = 0,binding = 2) uniform subpassInput GBuffer2;

void main(){

//场景所有对象绘制为白色
	vec4 bufferColor1 = subpassLoad(GBuffer1);
	vec4 bufferColor2 = subpassLoad(GBuffer2);
	outColor = vec4(bufferColor1.xyz + bufferColor2.xyz ,1);

}