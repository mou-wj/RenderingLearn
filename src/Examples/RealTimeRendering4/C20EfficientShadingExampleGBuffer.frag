#version 450 core


layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor1;
layout(location = 1) out vec4 outColor2;
void main(){

//场景所有对象绘制为白色
	outColor1 = vec4(1,0,0,1);
	outColor2 = vec4(0,1,0,1);
}