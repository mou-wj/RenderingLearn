#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outPosition;//世界空间中的位置
layout(location = 1) out vec3 outNormal;//世界空间中的法向量
layout(location = 2) out vec3 outTexCoord;//纹理坐标u,v,w
layout(set = 0,binding = 0,std140) uniform SimpleSceenExampleBuffer{
	mat4 world;
	mat4 view;
	mat4 proj;
};
void main(){
	gl_Position = proj * view * world *vec4(inPosition,1.0f);
	vec4 tmp = world * vec4(inPosition,1.0f);
	tmp/=tmp.w;
	outPosition = tmp.xyz;
	mat4 normalTransformMatrix = transpose(inverse(world));
	outNormal = normalize(vec3(normalTransformMatrix * vec4(normalize(inNormal),0.0f)));

	outTexCoord = inTexCoord;


}