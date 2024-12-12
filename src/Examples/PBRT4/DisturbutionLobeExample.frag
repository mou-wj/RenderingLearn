#version 450 core


layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor;

void main(){

//场景所有对象绘制为白色
	outColor = vec4(inColor,1);

}