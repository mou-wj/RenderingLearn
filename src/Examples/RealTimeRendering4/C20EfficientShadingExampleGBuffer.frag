#version 450 core


layout(location = 0) in vec3 inColor;
layout(rgba32f,set = 0,binding = 2) uniform image2D GBuffer1;//存放有深度信息

layout(location = 0) out vec4 outColor1;
void main(){

	//场景所有对象绘制为白色
	outColor1 = vec4(1,0,0,1);

	ivec2 fcoord = ivec2(gl_FragCoord.xy);
	imageStore(GBuffer1,fcoord,vec4(1,1,1,gl_FragCoord.z));
}