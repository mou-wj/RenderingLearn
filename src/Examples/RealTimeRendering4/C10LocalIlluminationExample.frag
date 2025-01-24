#version 450 core

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的法线
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;


//定义几个面光源
struct RectLight{
	vec3 pos;
	vec3 rangeMin,rangeMax;
	vec3 normal;
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


layout(set = 0,binding = 1) uniform sampler2D preComputeTexture;


layout(set = 0,binding = 3,std140) uniform Info{
	layout(offset = 16) vec3 cameraPos;//世界空间中相机位置
	layout(offset = 32) int exampleType;//示例: 0表示GGX的镜面反射示例; 1表示光滑次表面散射模型;2表示粗糙次表面散射迪士尼模型;3表示粗糙次表面散射OrenNayar模型;4表示经验布料模型;5表示微表面布料模型
};





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
	vec3 l = normalize(pointLightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float hDotL = dot(h,l);
	float hDotV = dot(h,v);
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);
	float nDotH = dot(h,inNormal);
	vec3 fdiff = Pss / s_pi;
	vec3 cdiff = s_pi * fdiff * pointLightColor * max(nDotL,0);

	float shininess = 5;
	float fspec = pow(max(nDotH, 0.0), shininess); 
	vec3 cspec = vec3(1,1,1) * (1 - Pss) * fspec;



	outColor = vec4(cdiff + cspec,1.0);



}


void main(){
	outColor = vec4(1,0,0,1);
	vec3 color = texture(preComputeTexture,vec2(0,0)).xyz;


	//GGXSpecularExample();
	BlinnPhongExample();


}