#version 450 core


layout(location = 0) in vec3 inSampleVec;
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
	
	uint numSample = 16;
	
	for(uint sampleIndex =0;sampleIndex< numSample;sampleIndex++)
	{
		float trans = 0;//��ǰ��͸����
		//��ȡһ��delta_t



		
	
	
	}





}

void main(){

//�������ж������Ϊ��ɫ
	outColor = vec4(1,1,1,1);

}