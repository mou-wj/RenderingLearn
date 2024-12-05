#version 450 core


layout(set = 0,binding = 1) uniform samplerCube testTexture;

layout(location = 0) in vec3 inSampleVec;
layout(location = 0) out vec4 outColor;

void main(){

	//����glsl�Ĳ��������½���Ϊ����ԭ��(0,0),�����������ɲ��������ʱ��texture�������������ʱ���ڲ��ᷴת����v������������һЩ����»ᵼ�����µߵ�
	vec3 sampleV = inSampleVec * vec3(1.0,1.0,1.0);
	vec3 texColor = texture(testTexture,sampleV).xyz;
	//gama ����
	texColor = pow(texColor, vec3(2.4));

	outColor = vec4(texColor,1.0);
}