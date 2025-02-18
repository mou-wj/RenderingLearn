#version 450 core

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��еķ���
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;


vec3 pointLightPos = vec3(0,-3,6);
vec3 pointLightColor = vec3(1,0,0);
const float s_pi = 3.141592653;

const vec3 Pss = vec3(0.3);

layout(set = 0,binding = 3,std140) uniform Info{
	layout(offset = 16) vec3 cameraPos;//����ռ������λ��
	
};


void CottonShadeExample(){
	vec3 color = vec3(1,1,0);//����ɫΪ��ɫ
	//�����Դ�������λ��
	vec3 l = normalize(cameraPos-inPosition);
	float nDotL = max(dot(l,inNormal),0);

	if(nDotL >0.8)
	{
		color = color;
	}else if(nDotL >0.5){
		color = 0.5 * color;
	}else if(nDotL > 0.2)
	{
		color = 0.2 * color;
	}else {
		color = 0.05 * color;
	
	}



	outColor = vec4(color,1.0);



}

void ContourEdgeShadeExample(){

	vec3 v = normalize(cameraPos-inPosition);
	float nDotV = dot(v,inNormal);
	vec3 color = vec3(1,1,1);
	if(nDotV <0.1 && nDotV >=0.0)
	{
		outColor = vec4(color * nDotV,1.0);
	
	}


}

void main(){
	outColor = vec4(1,0,0,1);

	CottonShadeExample();
	ContourEdgeShadeExample();
}