#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 in1;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 in3;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outColor;
void main(){
	gl_Position =vec4(inPosition * 0.5f,1.0f);
	outColor = vec3(1.0,1.0,0.0);
}