#version 450 core

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的法线
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;

vec3 pointLightPos = vec3(0,-3,6);
vec3 pointLightColor = vec3(1,0,0);
const float s_pi = 3.141592653;

const vec3 Pss = vec3(0.3);


layout(set = 0,binding = 1) uniform sampler2D preComputeTexture;


layout(set = 0,binding = 3,std140) uniform Info{
	layout(offset = 16) vec3 cameraPos;//世界空间中相机位置
	layout(offset = 32) int exampleType;//示例: 0表示GGX的镜面反射示例; 1表示光滑次表面散射模型;2表示粗糙次表面散射迪士尼模型;3表示粗糙次表面散射OrenNayar模型;4表示经验布料模型;5表示微表面布料模型
};

// GLSL实现erf误差函数
float erf(float x) {
    // 常用近似公式（Abramowitz and Stegun公式 7.1.26）
    // erf(x) ≈ 1 - (a1*t + a2*t^2 + a3*t^3 + a4*t^4 + a5*t^5)*exp(-x^2)
    // 其中 t = 1 / (1 + p * x)
    const float a1 = 0.254829592;
    const float a2 = -0.284496736;
    const float a3 = 1.421413741;
    const float a4 = -1.453152027;
    const float a5 = 1.061405429;
    const float p = 0.3275911;

    // 确保 x 为正（误差函数是奇函数：erf(-x) = -erf(x)）
    float sign = x < 0.0 ? -1.0 : 1.0;
    x = abs(x);

    // Abramowitz and Stegun公式实现
    float t = 1.0 / (1.0 + p * x);
    float y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);

    return sign * y;
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

	float f_spec = frenel * g2Smith * ndf_ggx / ( 4 * abs(nDotL) * abs(nDotV) );

	vec3 specColor = f_spec * pointLightColor * max(hDotL,0);

	vec3 surface_color = vec3(0.2,0.2,0.2);
	surface_color +=specColor;

	outColor = vec4(surface_color,1.0);

}

//带多次弹射项的示例，目前由于预计算部分出现问题，暂时搁置
void GGXSpecularMultiBounceExample(){}


//光滑表面的次表面散射模型
void SmoothSurfaceSubsurfaceModelDiffuseExample(){
		//由于目前的示例只有一个光源，所以不需要采样
	vec3 l = normalize(pointLightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);

	float roughness = 0.1;
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);

	float eta = 1.5;
	float F0 = pow((eta - 1)/(eta + 1),2); 

	float facl = 1 - pow(1 - max(nDotL,0) ,5);
	float facv = 1 - pow(1 - max(nDotV,0) ,5);
	vec3 fdiff = 20.0 * (1 - F0) * facl * facv  * Pss / (21.0 * s_pi);
	vec3 difColor = pointLightColor * fdiff * max(nDotL,0);
	outColor = vec4(difColor,1);



}

//粗糙表面的次表面迪斯尼散射模型
void RoughnessSurfaceSubsurfaceModelDisneyExample(){

		//由于目前的示例只有一个光源，所以不需要采样
	vec3 l = normalize(pointLightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);
	float hDotL = dot(l,h);
	float kss = 0.3;
	
	float FD90 = 0.5 + 2 * sqrt(roughness) * pow(hDotL,2);;
	float FSS90 = sqrt(roughness) * pow(hDotL,2);
	
	float FSS = (1 + (FSS90 - 1) * pow(1 - nDotL,5)) * (1 + (FSS90 - 1) * pow(1 - nDotV,5));

	float fd = (1 + (FD90 - 1) * pow(1 - nDotL,5)) * (1 + (FD90 - 1) * pow(1 - nDotV,5));
	float fss = (1 / (nDotL * nDotV) - 0.5) * FSS + 0.5;

	vec3 fdiff = max(nDotL,0) * max(nDotV,0) * Pss * ((1 - kss)* fd + 1.25 *  kss * fss  ) / s_pi;
	vec3 difColor = pointLightColor * fdiff * max(nDotL,0);
	outColor = vec4(difColor,1);


}


//粗糙表面的次表面Oren-Nayar散射模型
void RoughnessSurfaceSubsurfaceModelOrenNayarExample(){

		//由于目前的示例只有一个光源，所以不需要采样
	vec3 l = normalize(pointLightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);
	float nDotH = dot(h,inNormal);
	float lDotV = dot(l,v);
	float eta = 1.5;
	float F0 = pow((eta - 1)/(eta + 1),2); 
	
	float kfacing = 0.5 + 0.5 * lDotV;
	float fmulti = 0.3641 * roughness;

	float fsmooth = 21.0 * (1 - F0) * (1 - pow(1 - nDotL,5)) * (1 - pow(1 - nDotL,5)) / 20.0; 
	float frough = kfacing * (0.9 - 0.4 * kfacing) * ((0.5 + nDotH) / nDotH);


	vec3 fdiff = max(nDotL,0) * max(nDotV,0) * Pss * ((1 - roughness)* fsmooth + roughness * frough + fmulti * Pss  ) / s_pi;
	vec3 difColor = pointLightColor * fdiff * max(nDotL,0);
	outColor = vec4(difColor,1);


}

//经验布料模型
void FabricModelExample(){

		//由于目前的示例只有一个光源，所以不需要采样
	vec3 l = normalize(pointLightPos - inPosition);
	float nDotL = dot(l,inNormal);
	float w = 0.3;
	vec3 cscatter = vec3(0,0.8,0.3);

	vec3 s = cscatter + max(nDotL,0);
	s = vec3(max(s.x,0),max(s.y,0),max(s.z,0));



	vec3 fdiff = s * Pss * max(nDotL + w,0) / (s_pi * (1 + w));
	vec3 difColor = pointLightColor * fdiff;
	outColor = vec4(difColor,1);



}



//天鹅绒的NDF
float NDF_Velvet(float nDotM,float roughness){
	float res;

	float kamp = 0.5;
	float nDotM2 = pow(nDotM,2);
	float roughness2 = pow(roughness,2);

	float fac1 = max(nDotM,0) / ( ( 1 + kamp * pow(roughness,2) ) * s_pi );
	float fac2 = 1 + kamp * exp( nDotM2 / ( roughness2 * (nDotM2 - 1) ) ) / pow( 1 - nDotM2  ,2);

	res = fac1 * fac2;

	return res;
}


//微表面布料模型
void MicrofacetFabricModelExample(){
	
		//由于目前的示例只有一个光源，所以不需要采样
	vec3 l = normalize(pointLightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);
	float nDotH = dot(h,inNormal);
	float lDotH = dot(h,l);
	float lDotV = dot(l,v);
	float eta = 1.5;
	
	float frenel = Frenel(lDotH,eta);
	vec3 fac1 = (1 - frenel) * Pss / s_pi;
	float fac2 = frenel * NDF_Velvet(nDotH,roughness) / ( 4 * ( nDotV + nDotL - nDotL * nDotV ) );

	vec3 fdiff = fac1 + fac2;
	vec3 difColor = pointLightColor * fdiff * max(nDotL,0);
	outColor = vec4(difColor,1);

}




void main(){
	outColor = vec4(1,0,0,1);
	vec3 color = texture(preComputeTexture,vec2(0,0)).xyz;

	if(exampleType == 0)
	{
		GGXSpecularExample();
	}else if(exampleType == 1){
		SmoothSurfaceSubsurfaceModelDiffuseExample();
	}else if(exampleType == 2){
		RoughnessSurfaceSubsurfaceModelDisneyExample();
	}else if(exampleType == 3){
		RoughnessSurfaceSubsurfaceModelOrenNayarExample();
	}else if(exampleType == 4){
		FabricModelExample();
	}else if(exampleType == 5){
		MicrofacetFabricModelExample();
	}


	//GGXSpecularExample();
	//SmoothSurfaceSubsurfaceModelDiffuseExample();
	//RoughnessSurfaceSubsurfaceModelDisneyExample();
	//RoughnessSurfaceSubsurfaceModelOrenNayarExample();
	//FabricModelExample();
	//MicrofacetFabricModelExample();
}