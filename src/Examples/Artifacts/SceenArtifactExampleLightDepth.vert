#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outTexCoord;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec3 outPosition1;
layout(set = 0,binding = 0,std140) uniform TransformBuffer{
	mat4 world;
	mat4 view;
	mat4 proj;
};
void main(){
	gl_Position = proj * view * world *vec4(inPosition,1.0f);
	outTexCoord = inTexCoord;
	gl_Position/=gl_Position.w;
	outPosition = gl_Position.xyz;
	outPosition1 = inPosition;
}