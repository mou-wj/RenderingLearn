#version 450 core


layout(location = 0) in vec3 inSampleVec;
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

//根据位置返回一个随机的0-1的数
float random(vec3 pos) {
    return fract(sin(dot(pos, vec3(12.9898, 78.233,55.66))) * 43758.5453);
}

//定义sigma_t  ,根据位置随机产生一个sigma_t   ，sigma_t = sigma_a(衰减项) + sigma_s(散射项)
float RandomSigma_t(vec3 pos){
	return random(pos);
}



//phase function 类似pdf
float PhaseFunctionConstant(){
	return 1 / ( 4 * s_pi);
}

//Henyey–Greenstein Phase Function   类似pdf
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
		float trans = 0;//当前的透射项
		//获取一个delta_t



		
	
	
	}





}

void main(){

//场景所有对象绘制为白色
	outColor = vec4(1,1,1,1);

}