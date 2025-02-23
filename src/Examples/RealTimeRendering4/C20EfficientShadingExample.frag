#version 450 core


layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor;

layout(rgba32f,set = 0,binding = 2) uniform image2D GBuffer1;//����������Ϣ

void main(){

//�������ж������Ϊ��ɫ
	vec4 bufferColor1 = imageLoad(GBuffer1,ivec2(gl_FragCoord.xy));
			//gama ����
	bufferColor1 = pow(bufferColor1, vec4(2.4));
	outColor = vec4(bufferColor1.xyz,1);

}