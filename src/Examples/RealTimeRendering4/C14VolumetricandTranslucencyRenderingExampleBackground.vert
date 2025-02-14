#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outPosition;

layout(set = 0,binding = 0,std140) uniform SimpleSceenExampleBuffer{
	mat4 world;
	mat4 view;
	mat4 proj;
};
void main(){

	//取消view矩阵中的位移分量
	mat4 fixView = mat4(mat3(view));
	outPosition = inPosition * 10;
	gl_Position = proj * fixView * world *vec4(inPosition,1.0f);
	gl_Position.z = gl_Position.w;
}