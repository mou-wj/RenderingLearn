#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(set = 0,binding = 0,std140) uniform SimpleSceenExampleBuffer{
	mat4 world;
	mat4 view;
	mat4 proj;
};

layout(location = 0) out vec3 outViewPosition;//相机空间中的位置 

void main(){
	gl_Position = proj * view * world *vec4(inPosition,1.0f);
	vec4 viewWorldPosition = view * world * vec4(inPosition,1.0);
	viewWorldPosition /=viewWorldPosition.w;
	outViewPosition = viewWorldPosition.xyz;
}