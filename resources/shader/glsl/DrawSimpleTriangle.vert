#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 outColor;
void main(){
	gl_Position =vec4(inPosition * 0.5f,1.0f);
	outColor = vec3(1.0,1.0,0.0);
}