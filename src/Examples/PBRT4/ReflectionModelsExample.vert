#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 in3;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outWorldPosition;
layout(set = 0,binding = 0,std140) uniform TransferBuffer{
	mat4 world;
	mat4 view;
	mat4 proj;
};
void main(){
	gl_Position = proj * view * world *vec4(inPosition,1.0f);
	outNormal = normalize(mat3(world) * inNormal);//法线变换到世界坐标系中
	vec4 worldP = world *vec4(inPosition,1.0f);
	outWorldPosition = (worldP / worldP.w).xyz;//位置变换到世界坐标系
}