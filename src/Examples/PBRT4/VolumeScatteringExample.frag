#version 450 core

//to do��һ�������е�࣬��ʱ������������ʹ�ù���׷����ɫ����ʵ��

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

//����λ�÷���һ�������0-1����
float random(vec3 pos) {
    return fract(sin(dot(pos, vec3(12.9898, 78.233,55.66))) * 43758.5453);
}

//����sigma_t  ,����λ���������һ��sigma_t   ��sigma_t = sigma_a(˥����) + sigma_s(ɢ����)
float RandomSigma_t(vec3 pos){
	return random(pos);
}



//phase function ����pdf
float PhaseFunctionConstant(){
	return 1 / ( 4 * s_pi);
}

//Henyey�CGreenstein Phase Function   ����pdf
float PhaseFunctionHenyeyGreenstein(float cosTheta, float g){
    float denom = 1 + pow(g,2) + 2 * g * cosTheta;
	denom = pow(denom,1.5) * 4 * s_pi;
	return (1 - pow(g,2)) / denom;
}
vec3 BoundMin,BoundMax;
void VolumeScattering(vec3 wo,vec3 normal){
	
	uint numSample = 20;
	float sigma_major =1;
	float sigma_null=0;
	//���㷨������ϵ
	vec3 y = -normalize(normal);
	vec3 x,z;
	if((1 - abs(y.x)) > 0.00000001)
	{
		x = vec3(1,0,0);
	}else {
		x = vec3(0,1,0);
	}
	z = normalize(cross(x,y));
	x = normalize(cross(y,z));
	mat3 normalMatrix = mat3(x,y,z);

	//����Ƭ��������ռ��λ�úͱ߽��Լ��ӽǷ���ȷ���ܲ���t
	wo = normalize(wo);

	//�̶�ģ��Ϊһ�������壬�̶��ӽ����������壬���ӽǺͱ߽�͵�ǰ����������֮��ľ���Ϊ2
	float d = 2;






	float trans = 0;//��ǰ��͸����
	for(uint sampleIndex =0;sampleIndex< numSample;sampleIndex++)
	{
		
		//��ȡһ��delta_t
		float deltaT = d / numSample;

		float sigma_t = random(inPosition - deltaT * wo);
		sigma_null = sigma_major - sigma_t;

		trans+=sigma_null/sigma_major;
	
	}





}

void main(){

//�������ж������Ϊ��ɫ
	outColor = vec4(1,1,1,1);

}