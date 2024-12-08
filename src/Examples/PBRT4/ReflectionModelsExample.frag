#version 450 core

const float s_pi = 3.141592653;

layout(location = 0) in vec3 inNormal;//输入世界空间中的法线
layout(location = 1) in vec3 inWorldPosition;//输入世界空间中的位置


layout(location = 0) out vec4 outColor;


layout(set = 0,binding = 1) uniform samplerCube skyTexture;
layout(set = 0,binding = 2) uniform SceenInfo{//场景信息
	vec3 viewPosition;//相机在世界坐标系中的位置

};


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



//BSDF 为BRDF和BTDF的统称


//简单反射模型 Diffuse Reflection  Lambert model 漫反射
//Diffuse Reflection


float s_LamberRefectionFactor = 0.2;//lambert模型反射率
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
	vec3 reflectLight = vec3(0);
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
	z = normalize(cross(x,y));
	x = normalize(cross(y,z));
	mat3 nornalMatrix = mat3(x,y,z);
	float totalPDF = 0;
	float pdf = 0;
	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//获取二维随机点，为（theta，phi）
		samplePoint = HaltonSample2D(sampleIndex); 
		//将随机点转化为半球中的方向
		float theta = samplePoint.x * 2* s_pi;
		float fine = 0.5 * s_pi * samplePoint.y;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = nornalMatrix * wi;//变换到世界坐标
		wi  = normalize(wi);
		light = texture(skyTexture,wi).xyz;
		//gama 解码，转线性空间
		light = pow(light, vec3(2.4));
		pdf = PDFLambert(wi,wo);
		reflectLight+= BSDFLambert(wo,wi) * light * dot(n,wi) * pdf;
		totalPDF+=pdf;

	}
	reflectLight /= totalPDF;//归一化
	return reflectLight;

}


//frenel 折射 Ni * sin(theta_i) = No * sin(theta_o)  其中Ni No表示各自的折射率，theta_i theta_o表示各自和法线之间的夹角
//两种材质，其入射光Li，出射光Lo满足pow(Ni,2) * Lo = pow(No,2) * Li， 其 BTDF 满足 pow(Ni,2) * BTDF(p,wi,wo) = pow(No,2) * BTDF(p,wo,wi) 


//简单镜面反射模型
const float specularReflectionFactor = 0.8;

vec3 BRDFSimpleSpecular(vec3 wo,vec3 wi)
{
	vec3 res;
	res.x = specularReflectionFactor;
	res.y = specularReflectionFactor;
	res.z = specularReflectionFactor;
	return res;
	
}

const float speclarFactor = 30;//控制高光的情况

float PDFSimpleSpecular(float NdotH/*法线和半角之间的cos值*/){
	float res;
	//简单blinn-phong镜面反射pdf
	float fac = pow(NdotH,speclarFactor);
	res = (speclarFactor + 2)  *  fac / (2 * s_pi);
	return res;
}

//有一定粗糙度的镜面反射
vec3 RoughnessSpecularBRDF(vec3 wo, vec3 n){
	uint numSample = 16;
	vec2 samplePoint;
	vec3 wi;
	vec3 light;
	vec3 reflectLight = vec3(0);
	vec3 halfVec;

	float dw = 2 * s_pi / numSample;
	vec3 y = -normalize(n);
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

	float nDotH = 0,pdf=0,nDotWi =0 ;
	float totalPDF = 0;

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//获取二维随机点，为（theta，phi）
		samplePoint = HaltonSample2D(sampleIndex); 
		//将随机点转化为半球中的方向
		float theta = samplePoint.x * 2* s_pi;
		float fine = 0.5 * s_pi * samplePoint.y;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalMatrix * wi;//变换到世界坐标
		wi  = normalize(wi);
		light = texture(skyTexture,wi).xyz;
		//计算half半向量
		halfVec = normalize(wi+wo);
		nDotH = clamp(dot(halfVec,n),0,1);
		pdf = PDFSimpleSpecular(nDotH);


		//gama 解码，转线性空间
		light = pow(light, vec3(2.4));
		if(pdf > 0.0001)
		{
			nDotWi = clamp(dot(n,wi),0,1);
			vec3 curLight = BRDFSimpleSpecular(wo,wi) * light * nDotWi;
			reflectLight+=  curLight * pdf;//这里不是求的所有光的积分总和，而是求的每个光应该在最终的结果中占据的比例，所以不能除以pdf
			totalPDF +=pdf;
		
		}
	}
	//reflectLight /= numSample;
	reflectLight /=  totalPDF;//归一化
	return reflectLight;

}


vec3 SimpleSpecularBRDF(vec3 wo, vec3 n){
	vec3 wi = normalize(reflect(-wo,n));
	vec3 light = texture(skyTexture,wi).xyz;
	float nDotWi = clamp(dot(n,wi),0,1);
	return light * nDotWi * specularReflectionFactor;
}

const float refractFactor = 0.8;
vec3 SimpleBTDF(vec3 wo, vec3 n){
	
	vec3 wi = normalize(refract(-wo,n,1 / 1.5));
	vec3 light = texture(skyTexture,wi).xyz;
	float nDotWi = clamp(dot(-n,wi),0,1);
	return light * nDotWi * specularReflectionFactor;

}


//微表面理论来表示粗糙度





void main(){

	vec3 wo = normalize(viewPosition - inWorldPosition);
	vec3 color;
	//color = LambertBSDF(wo,inNormal);
	//color += RoughnessSpecularBRDF(wo,inNormal);
	//color = SimpleSpecularBRDF(wo,inNormal);
	color = SimpleBTDF(wo,inNormal);
	//gama 解码
	//color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);



	//场景所有对象绘制为白色
	//outColor = vec4(1,1,1,1);


}