#version 450 core

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的法线
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;


//定义几个面光源
struct RectLight{
	vec3 p1,p2,p3,p4;
	vec3 normal;
	vec3 color;
};


struct SphereLight{
	vec3 pos;
	vec3 color;
	vec3 radius;
};


struct PipeLight{
	vec3 pos1,pos2;
	vec3 color;
};


vec3 pointLightPos = vec3(0,-3,6);
vec3 pointLightColor = vec3(1,0,0);
const float s_pi = 3.141592653;

const vec3 Pss = vec3(0.3);

layout(set = 0,binding = 2) uniform sampler2D envTexture;

layout(set = 0,binding = 3,std140) uniform Info{
	layout(offset = 16) vec3 cameraPos;//世界空间中相机位置
	layout(offset = 32) int exampleType;//示例: 0表示GGX的镜面反射示例; 1表示光滑次表面散射模型;2表示粗糙次表面散射迪士尼模型;3表示粗糙次表面散射OrenNayar模型;4表示经验布料模型;5表示微表面布料模型
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

// 计算三维空间中的双线性插值
vec3 bilinearInterpolate(vec3 A, vec3 B, vec3 C, vec3 D, float u, float v) {
    // 双线性插值公式
    return (1.0 - u) * (1.0 - v) * A + u * (1.0 - v) * B + (1.0 - u) * v * D + u * v * C;
}


//计算矩形光源在p处的光源向量Ep
void CalculateRectLightVec(in vec3 p,out vec3 Ep,out float LenEp){
	
	vec3 l = vec3(0);
	uint numSample = 20;
	RectLight rectLight;
	rectLight.p1 = vec3(-4,-3,5);
	rectLight.p2 = vec3(-4,-3,7);
	rectLight.p3 = vec3(4,-3,7);
	rectLight.p4 = vec3(4,-3,5);
	

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		vec2 samplePoint = HaltonSample2D(sampleIndex);
		
		vec3 sampleLightPosition = bilinearInterpolate(rectLight.p1,rectLight.p2,rectLight.p3,rectLight.p4,samplePoint.x,samplePoint.y);

		vec3 curLi = normalize(sampleLightPosition - p);

		//计算l长度
		float d= distance(sampleLightPosition ,p);
		curLi *= 1 / pow(d,2);

		//考虑距离衰减来作为分布
		l += curLi;

	}
	LenEp = length(l) / numSample;
	Ep = normalize(l);
}


//估计矩形光源在p处的光源向量Ep
void DrobotEstimateRectLightVec(in vec3 p,in vec3 n,out vec3 L/*估计的面光源的方向*/,out float LenL){
	
	vec3 l = vec3(0);
	uint numSample = 400;
	RectLight rectLight;
	rectLight.p1 = vec3(-4,-3,5);
	rectLight.p2 = vec3(-4,-3,7);
	rectLight.p3 = vec3(4,-3,7);
	rectLight.p4 = vec3(4,-3,5);
	
	float maxFac = 0;

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		vec2 samplePoint = HaltonSample2D(sampleIndex);
		
		vec3 sampleLightPosition = bilinearInterpolate(rectLight.p1,rectLight.p2,rectLight.p3,rectLight.p4,samplePoint.x,samplePoint.y);

		vec3 curLi = normalize(sampleLightPosition - p);
		float nDotL = max(dot(curLi,n),0);
				//计算l长度
		float d= distance(sampleLightPosition ,p);
		float fac = nDotL / pow(d,2);

		//找到因子最大的方向并记录
		if(fac > maxFac)
		{
			maxFac = fac;
			L= curLi;
			LenL =  1 / pow(d,2);
		}
	}
}



float Frenel(float nDotL,float eta/*相对折射率，入射的介质折射率/入射光所在的介质折射率 */){
	float F0 = pow((eta - 1)/(eta + 1),2); 
	float r = F0 + (1- F0) * pow(1 - max(nDotL,0),5);
	return r;

}



//各项同性的GGX PDF
float NDF_GGX_Isotropy(float nDotM,float roughness){
	float res = 0;
	float numerator = max(nDotM,0) * pow(roughness,2);
	float denominator = s_pi * pow( 1 + pow(nDotM,2) * (pow(roughness,2) - 1) ,2);

	res = numerator / denominator;
	return res;
}

//各项同性的GGX Lambda函数
float LambdaGGX_Isotropy(float nDotS,float roughness){
	float res = 0;
	float a = nDotS / (roughness * sqrt( 1 - pow(nDotS,2) ));
	res = (sqrt(1 + 1 / pow(a,2)) - 1) / 2; 
	return res;


}

//各项异性的GGX PDF
float NDF_GGX_Anisotropy(float nDotM,float tDotM/*在纹理空间中m与t的叉积*/,float bDotM/*在纹理空间中m与b的叉积*/,float roughnessX,float roughnessY){
	float res = 0;
	float numerator = max(nDotM,0);
	float denominator = s_pi * roughnessX * roughnessY * pow( pow(nDotM,2) + pow(tDotM / roughnessX,2) + pow(bDotM / roughnessY,2)  ,2);

	res = numerator / denominator;
	return res;
}



//mask函数
float G1(vec3 m,vec3 v,float lambdaV/*v视角的lambda函数结果*/){
	float mDotV = max(dot(m,v),0);
	float res = mDotV / ( 1 + lambdaV);
	return res;
}

//shadow和mask函数
float G2Smith(float mDotV,float mDotL,float lambdaV/*v视角的lambda函数结果*/,float lambdaL/*l视角的lambda函数结果*/){
	mDotV = max(mDotV,0);
	mDotL = max(mDotL,0);
	float res = mDotV * mDotL / ( 1 + lambdaV + lambdaL);
	return res;
}


//使用GGX分布实现一个只有镜面反射的反射模型
void GGXSpecularExample(){
	//由于目前的示例只有一个光源，所以不需要采样
	vec3 l = normalize(pointLightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float hDotL = dot(h,l);
	float hDotV = dot(h,v);
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);

	float lambdaV = LambdaGGX_Isotropy(hDotV,roughness);
	float lambdaL = LambdaGGX_Isotropy(hDotL,roughness);


	float ndf_ggx = NDF_GGX_Isotropy(dot(inNormal,h),0.1);
	float frenel =Frenel(nDotL,1.5);
	float g2Smith = G2Smith(hDotV,hDotL,lambdaV,lambdaL);

	float f_spec =  frenel * g2Smith * ndf_ggx / ( 4 * abs(nDotL) * abs(nDotV) );

	vec3 specColor = f_spec * pointLightColor * max(hDotL,0);

	vec3 surface_color = vec3(0.2,0.2,0.2);
	surface_color +=specColor;

	outColor = vec4(surface_color,1.0);

}


//lambdert 光照模型
void BlinnPhongExample(){
	//由于目前的示例只有一个光源，所以不需要采样



	vec3 l;
	l = normalize(pointLightPos - inPosition);
	float d = distance(pointLightPos,inPosition);
	float curD = 1 / pow(d,2);
	vec3 Ep;
	float LenEp;
	
	CalculateRectLightVec(inPosition,Ep,LenEp);
	l = Ep;
	curD = LenEp;

	vec3 L;
	float LenL;
	//这种方法会由于采样的随机性导致最终得到的高光结果和预期结果存在偏差从而导致结果出问题
	//DrobotEstimateRectLightVec(inPosition,inNormal,L,LenL);
	//l = L;
	//curD = LenL;
	
	//LenEp = curD;

	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float hDotL = dot(h,l);
	float hDotV = dot(h,v);
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);
	float nDotH = dot(h,inNormal);
	vec3 fdiff = Pss / s_pi;
	vec3 cdiff = s_pi * fdiff * pointLightColor * max(nDotL,0) * curD;

	float shininess = 30;
	float fspec = pow(max(nDotH, 0.0), shininess); 
	vec3 cspec = vec3(1,1,1) * (1 - Pss) * fspec;



	outColor = vec4(cdiff + cspec ,1.0);



}


void SphereCoordEnvMapExample(){
	

	vec3 n = normalize(inPosition);
	//计算u
	float u  = (atan(n.z,n.x) / (s_pi) + 1) / 2;
	float v  = (asin(n.y) / (0.5 * s_pi) + 1) / 2;
	vec3 color = texture(envTexture,vec2(u,v)).xyz;

	color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);
	

}


void main(){
	outColor = vec4(1,0,0,1);
//	if(exampleType == 0)
//	{
//		BlinnPhongExample();
//	}else if(exampleType == 1){
//		SphereCoordEnvMapExample();
//	}
	SphereCoordEnvMapExample();
	//EnvMapExample();


}