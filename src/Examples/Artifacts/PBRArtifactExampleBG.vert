#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outPosition;
layout(set = 0,binding = 0,std140) uniform TransformBuffer{
	mat4 world;
	mat4 view;
	mat4 proj;
};
void main(){
	//��պе�ģ�Ͳ���Ҫλ�ƣ�ֻ��Ҫ��ת����
	mat4 viewRotate = mat4(mat3(view));
	gl_Position = proj * viewRotate *vec4(inPosition,1.0f);
	gl_Position= gl_Position.xyww;//
	outPosition = inPosition;
}