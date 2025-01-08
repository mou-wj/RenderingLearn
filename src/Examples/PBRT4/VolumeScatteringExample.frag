#version 450 core

//to do这一节问题有点多，暂时跳过，待后面使用光线追踪着色器来实现

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
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
	
	uint numSample = 20;
	float sigma_major =1;
	float sigma_null=0;
	//计算法线坐标系
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

	//根据片段在世界空间的位置和边界以及视角方向确定总步长t
	wo = normalize(wo);

	//固定模型为一个立方体，固定视角正对立方体，则视角和边界和的前后两个交点之间的距离为2
	float d = 2;






	float trans = 0;//当前的透射项
	for(uint sampleIndex =0;sampleIndex< numSample;sampleIndex++)
	{
		
		//获取一个delta_t
		float deltaT = d / numSample;

		float sigma_t = random(inPosition - deltaT * wo);
		sigma_null = sigma_major - sigma_t;

		trans+=sigma_null/sigma_major;
	
	}





}

void main(){

//场景所有对象绘制为白色
	outColor = vec4(1,1,1,1);

}