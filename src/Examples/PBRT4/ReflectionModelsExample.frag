#version 450 core

const float s_pi = 3.141592653;

layout(location = 0) in vec3 inNormal;//输入世界空间中的法线
layout(location = 0) out vec4 outColor;


layout(set = 0,binding = 1) uniform samplerCube skyTexture;


//生成随机数
// 生成Hammersley 序列的一维采样点，各个维度的序列可以使用不同的基来生成
float HammersleySequence(uint index, uint base) {
    float result = 0.0;
    float f = 1.0;
    uint i = index;
    while (i > 0) {
        f /= float(base);
        result += f * float(i % base);
        i /= base;
    }
    return result;
}

// 生成均匀分布的二维Halton采样点
vec2 HaltonSample2D(uint index) {
    return vec2(HammersleySequence(index, 2),HammersleySequence(index, 3)); // 基数选择 2,3
}





uint integrateType = 0;//0表示均匀采样积分，1表示重要性采样

//BSDF 为BRDF和BTDF的统称


//简单反射模型 Diffuse Reflection  Lambert model
//Diffuse Reflection


float s_LamberRefectionFactor = 0.4;//lambert模型反射率
float s_LambertxFactor  = s_LamberRefectionFactor / s_pi;


//返回出射光为wo情况下，入射光的概率密度
float PDFLambert(vec3 wi,vec3 wo)
{
	return 0.5/s_pi;
}

vec3 BSDFLambert(vec3 wi,vec3 wo){
	vec3 res;
	res.x = s_LambertxFactor;
	res.y = s_LambertxFactor;
	res.z = s_LambertxFactor;
	return res;
}

vec3 LambertBSDF(vec3 wo,vec3 n){
	
	uint numSample = 16;
	vec2 samplePoint;
	vec3 wi;
	vec3 light;
	vec3 reflectLight;
	float dw = 2 * s_pi / numSample;
	//根据世界空间的法线构建上半球坐标系
	vec3 y = -normalize(n);
	vec3 x,z;
	if((1 - abs(y.x)) > 0.00000001)
	{
		x = vec3(1,0,0);
	}else {
		x = vec3(0,1,0);
	}
	z = cross(x,y);
	x = cross(y,z);
	z = normalize(z);
	x = normalize(x);
	mat3 nornalMatrix;


	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//获取二维随机点，为（theta，phi）
		samplePoint = HaltonSample2D(sampleIndex); 
		//将随机点转化为半球中的方向
		wi.x = cos(samplePoint.x * 2* s_pi)* sin(0.5 * s_pi * samplePoint.y);
		wi.z = sin(samplePoint.x * 2* s_pi)* sin(0.5 * s_pi * samplePoint.y);
		wi.y = -sin(0.5 * s_pi * samplePoint.y);
		wi = nornalMatrix * wi;//变换到世界坐标
		light = texture(skyTexture,wi).xyz;
		if(integrateType == 0)
		{
			reflectLight+= BSDFLambert(wo,wi) * light * dot(n,wi) * dw;
		}else {
			reflectLight+= BSDFLambert(wo,wi) * light * dot(n,wi) / PDFLambert(wi,wo)/numSample;
		}


	}

	return reflectLight;

}







void main(){

//场景所有对象绘制为白色
	outColor = vec4(1,1,1,1);

}