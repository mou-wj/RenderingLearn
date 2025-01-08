#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 in3;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;//世界空间中的位置
layout(set = 0,binding = 0,std140) uniform SimpleSceenExampleBuffer{
	mat4 world;
	mat4 view;
	mat4 proj;
};
void main(){
	gl_Position = proj * view * world *vec4(inPosition,1.0f);
	vec4 tmpPos = world *vec4(inPosition,1.0f);
	outPosition = (tmpPos / tmpPos.w).xyz;
	outNormal = normalize(mat3(world) * inNormal);



}