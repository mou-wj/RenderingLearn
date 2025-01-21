#version 450 core

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��е�λ��
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;


layout(set = 0,binding = 3,std140) uniform Info{
	float animateDelta;
	layout(offset = 16) vec3 cameraPos;//����ռ������λ��
	layout(offset = 32) int exampleType;//Ҫ��ʾ��ʵ�����ͣ�0: ֱ�ӻ�ȡ����ֵ ; 1:mipmap ;2: SAT; 3:������ɵ�����;4:������;5:��������;6:alpha��ͼ
};

vec3 Frenel(float nDotL,float eta/*��������ʣ�����Ľ���������/��������ڵĽ��������� */){
	float F0 = pow((eta - 1)/(eta + 1),2); 
	float r = F0 + (1- F0) * pow(1 - max(nDotL,0),5);
	return vec3(r);

}




void main(){
	outColor = vec4(1,0,0,1);

}