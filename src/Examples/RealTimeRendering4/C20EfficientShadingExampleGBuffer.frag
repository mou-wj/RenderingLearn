#version 450 core


layout(location = 0) in vec3 inColor;
layout(rgba32f,set = 0,binding = 2) uniform image2D GBuffer1;//����������Ϣ

layout(location = 0) out vec4 outColor1;
void main(){

	//�������ж������Ϊ��ɫ
	outColor1 = vec4(1,0,0,1);

	ivec2 fcoord = ivec2(gl_FragCoord.xy);
	imageStore(GBuffer1,fcoord,vec4(1,1,1,gl_FragCoord.z));
}