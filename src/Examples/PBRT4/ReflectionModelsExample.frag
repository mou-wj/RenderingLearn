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

vec3 ExampleLambertBSDF(vec3 wo,vec3 n){
	
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
//两种材质，其入射光Li，出射光Lo满足pow(Ni,2) * Lo = pow(No,2) * Li， 其 BTDF 满足 pow(No,2) * BTDF(p,wi,wo) = pow(Ni,2) * BTDF(p,wo,wi) 


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
vec3 ExampleRoughnessSpecularBRDF(vec3 wo, vec3 n){
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


vec3 ExampleSimpleSpecularBRDF(vec3 wo, vec3 n){
	vec3 wi = normalize(reflect(-wo,n));
	vec3 light = texture(skyTexture,wi).xyz;
	float nDotWi = clamp(dot(n,wi),0,1);
	return light * nDotWi * specularReflectionFactor;
}

const float refractFactor = 0.8;
vec3 ExampleSimpleBTDF(vec3 wo, vec3 n){
	
	vec3 wi = normalize(refract(-wo,n,1 / 1.5));
	vec3 light = texture(skyTexture,wi).xyz;
	float nDotWi = clamp(dot(-n,wi),0,1);
	return light * nDotWi * specularReflectionFactor;

}


//微表面理论来表示粗糙度

float roughnessX = 0.3,roughnessY = 0.3;//GGX的theta，fine两个维度的粗糙度控制系数
//GGX 微表面分布，返回半圆内朝向w的的微表面的比率，传入的w处于局部空间,该分布和wo无关
float PDF_GGX(vec3 w)
{

	w = normalize(w);
	float theta = 0,fine = 0;
	if(-w.y == 0)
	{
		return 0;
	}
	fine = acos(-w.y); 
	theta = atan(w.z,w.x);
	float denominator = 1;
	denominator *= s_pi * roughnessX * roughnessY * pow(cos(fine),4);
	denominator *= pow(1 + pow(tan(fine),2) * (pow(cos(theta) / roughnessX,2) + pow(sin(theta)/ roughnessY,2)) ,2);
	return 1/denominator;

}

//遮挡，返回从wo方向看的微平面未被遮挡的比例
float UnMask(vec3 wo){
	wo = normalize(wo);
	float theta = 0,fine = 0;
	fine = acos(-wo.y); 
	if(-wo.y == 0)
	{
		return 0;
	}
	theta = atan(wo.z,wo.x);
	float denominator = 1;
	float roughness = sqrt(pow(roughnessX * cos(theta),2) + pow(roughnessY * sin(theta),2) );
	float lambdaW = (sqrt(1 + pow(roughness * tan(fine) ,2)) - 1) / 2;

	return 1 / (1+ lambdaW);
}

//返回从wo处看未被挡以及不在阴影中的比例
float UnMaskAndUnShadow(vec3 wo, vec3 wi){

	return UnMask(wo) * UnMask(wi);
}

//返回从wo处看未被挡以及不在阴影中的比例
float UnMaskAndUnShadow2(vec3 wo, vec3 wi){
	wo = normalize(wo);
	wi = normalize(wi);
	float theta = 0,fine = 0;
	float lambdaWo = 0;
	float lambdaWi = 0;
	fine = acos(-wo.y); 
	if(-wo.y != 0)
	{
		theta = atan(wo.z,wo.x);
		float roughnessWo = sqrt(pow(roughnessX * cos(theta),2) + pow(roughnessY * sin(theta),2) );
		lambdaWo = (sqrt(1 + pow(roughnessWo * tan(fine) ,2)) - 1) / 2;//计算wo的lambda辅助函数值
	}

	fine = acos(-wi.y); 
	if(-wi.y != 0)
	{
		theta = atan(wi.z,wi.x);
		float roughnessWi = sqrt(pow(roughnessX * cos(theta),2) + pow(roughnessY * sin(theta),2) );
		lambdaWi = (sqrt(1 + pow(roughnessWi * tan(fine) ,2)) - 1) / 2;//计算wi的lambda辅助函数值
	}

	return 1 / (1 + lambdaWi + lambdaWo);
}


//基于GGX构建一个和wo有关的微表面分布,该分布表示在观看角度为wo的情况下，朝向w的微表面比例
float PDF2_GGX(vec3 w,vec3 wo)
{
	float fine = acos(-wo.y);
	if(-wo.y == 0)
	{
		return 0;
	}
	float cosFine = cos(fine);

	return UnMask(wo) * PDF_GGX(w) * max(0,dot(w,wo)) / cos(fine);

}

//Torrance–Sparrow PDF 基于微表面分布，返回入射光wi的分布
float PDF_Sparrow(vec3 w,vec3 wo)
{
	float dotWoW = dot(w,wo);
	if(dotWoW == 0){
		return 0;
	}
	return PDF2_GGX(w,wo) / ( 4 * max(0,dotWoW));

}

//基于微表面分布，返回从wo处看，折射入射光wi的分布
float PDF_Refract(vec3 w,vec3 wo,float eta/*相对折射率: 界面的材质介质的折射率ni  / 出射光所在的介质的折射率no*/){
	//由于
	vec3 wi = reflect(-wo,w);
	wi = normalize(wi);
	//获取从wo处看反射光的pdf
	float pdf = PDF_Sparrow(w,wo);
	//基于反射光的分布计算折射入射光的分布   dwi/dwt = dwo/dwt
	float cosTheta_i = max(dot(wi,w),0);
	vec3 wt = refract(-wo,w,eta);
	float cosTheta_t = max(dot(wt,-w),0);
	if(cosTheta_i == 0)
	{
		return 0 ; 
	}
	float cosTheta_o = cosTheta_i;//出射光的cos值等于反射光的cos值
	cosTheta_i = cosTheta_t;//折射光作为入射光的cos值
	return pdf * pow(eta,2) * cosTheta_i / cosTheta_o;
}


//菲涅尔项  计算反射光的比例
float Frenel_Reflect(float cosTheta_i/*入射光和法线的夹角cos值*/,float eta/*相对折射率: 界面的材质介质的折射率 / 入射光所在的介质的折射率 */){
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	float cosTheta_t = sqrt(1-pow(sinTheta_i / eta ,2));
	float r_parellel = (eta *  cosTheta_i- cosTheta_t)  / (eta * cosTheta_i + cosTheta_t);
	float r_perpendicular = (cosTheta_i - eta * cosTheta_t)  / (cosTheta_i + eta * cosTheta_t);



	return (pow(r_parellel,2) + pow(r_perpendicular,2) ) / 2;
}

//菲涅尔项  简单计算折射光的比例 ，直接使用1减去菲涅尔计算的反射比例
float Refract_Simple_Frenel(float cosTheta_i/*入射光和法线的夹角cos值*/,float eta/*相对折射率: 界面的材质介质的折射率 / 入射光所在的介质的折射率 */){
	return 1 - Frenel_Reflect(cosTheta_i,eta);
}

float Ft_P_Wo_Wi(float fr_P_Wo_Wi){
	return 1-fr_P_Wo_Wi;
}

//根据入射光到折射光的btdf系数计算从折射光到入射光的btdf系数
//pow(No,2) * BTDF(p,wi,wo) = pow(Ni,2) * BTDF(p,wo,wi) 
float Ft_P_Wi_Wo(float ft_P_Wo_Wi/*入射光到材质*/,float eta/*相对折射率: 界面的材质介质的折射率no / 入射光所在的介质的折射率 ni*/)
{
	return pow(1/eta,2) * ft_P_Wo_Wi;
}

vec2 ComplexMultiply(vec2 complex1,vec2 complex2)
{
	return vec2(complex1.x * complex2.x - complex1.y * complex2.y , complex1.x * complex2.y + complex1.y * complex2.x );
}

//金属复数形式的frenel项
float Frenel_Reflect_Complex(float cosTheta_i/*入射光和法线的夹角*/,vec2 eta/*eta.x 表示相对折射率: 界面的材质介质的折射率 / 入射光所在的介质的折射率 ，eta.y表示衰减系数k， 该复数表示n+ik */){
	float n2Ak2 = pow(length(eta),2);
	float cosTheta_i2 = pow(cosTheta_i,2);
	float r_parellel = (n2Ak2 - 2 *eta.x * cosTheta_i + cosTheta_i2) / (n2Ak2 + 2 *eta.x * cosTheta_i + cosTheta_i2);
	float r_perpendicular = (n2Ak2 * cosTheta_i - 2 *eta.x * cosTheta_i + cosTheta_i2) / (n2Ak2 * cosTheta_i+ 2 *eta.x * cosTheta_i + cosTheta_i2);
    return (pow(r_parellel,2) + pow(r_perpendicular,2)) / 2;

}

//Sparrow 模型的BRDF示例
vec3 ExampleSparrowBRDT(vec3 wo,vec3 n)
{
	uint numSample = 100;
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
	mat3 normalMatrixInverse  = inverse(normalMatrix);

	float pdf=0,nDotWi =0 ;
	float totalPDF = 0,totalRefractPDF = 0;
	vec3 refractTotalLight = vec3(0);
	float eta = 1.5/1;
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
		wi = normalize(wi);
		//将wo转到局部空间
		vec3 localWo = normalMatrixInverse * normalize(wo); 
		//获得半向量
		halfVec = normalize(wi+localWo);

		//计算sparrow模型的 brdf项反射项
		
		pdf = PDF_Sparrow(halfVec,localWo);//wi的pdf
		float cosXXX = max(dot(halfVec,localWo),0);
		float frenel = Frenel_Reflect(max(dot(wi,vec3(0,-1,0)),0),eta) ;//菲涅尔项
		float unmask = UnMaskAndUnShadow2(localWo,wi);//非几何遮蔽和阴影项
		float curbrdf = pdf* frenel * unmask;

		//采样
		wi = normalMatrix * wi;//变换到世界坐标
		wi  = normalize(wi);
		light = texture(skyTexture,wi).xyz;

		//gama 解码，转线性空间
		light = pow(light, vec3(2.4));
		nDotWi = clamp(dot(n,wi),0,1);
		vec3 curLight = curbrdf * light * nDotWi;
		reflectLight+=  curLight * pdf;//这里不是求的所有光的积分总和，而是求的每个光应该在最终的结果中占据的比例，所以不能除以pdf
		totalPDF +=pdf;


		//计算折射的BTDF
		float btdf_p_wo_wi = 1-curbrdf;
		//计算从材质内部折射出来的光的btdf
		float btdf_p_wi_wo = Ft_P_Wi_Wo(btdf_p_wo_wi,eta);
		//计算折射光作为入射光的概率密度
		float pdf_refract = PDF_Refract(halfVec,localWo,eta);
		vec3 wi_refract = refract(-localWo,halfVec,eta);
		wi_refract = normalize(wi);
		if(length(wi_refract) == 0)
		{
			continue;
		}
		//将折射入射光变换到世界空间
		wi_refract = normalMatrix * wi_refract;//变换到世界坐标
		wi_refract  = normalize(wi_refract);
		light = texture(skyTexture,wi_refract).xyz;
		light = pow(light, vec3(2.4));
		refractTotalLight += pdf_refract * btdf_p_wi_wo * light;
		totalRefractPDF+= pdf_refract;
	}
	//reflectLight /= numSample;
	reflectLight /=  totalPDF;//归一化
	reflectLight *= (totalPDF) /(totalPDF + totalRefractPDF);
	refractTotalLight /= totalRefractPDF;//归一化
	refractTotalLight *= (totalRefractPDF) /(totalPDF + totalRefractPDF);

	//直接采样折射光
	//vec3 cur_ss = refract(-wo,n,1.2/1);
	//cur_ss  = normalize(cur_ss);
	//light = texture(skyTexture,cur_ss).xyz;
	//light = pow(light, vec3(2.4));

	vec3 resLight = reflectLight + refractTotalLight;
	return resLight;



}

//lambert漫反射模型，入射光被均匀反射到半圆的各个方向，其每个入射光的反射项f_p_wo_wi = R / PI,R为反射系数，表示入射的光有多少会被反射出去
float R_lambert = 0.8;//定义lambert的反射系数
//获取在出射光wo处，平面法线为n的情况下，lambert模型返回的出射光的大小
vec3 ReflectModelLambertDiffuseReflect(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){
	
	uint numSample = 100;
	vec2 samplePoint;
	vec3 wi;
	vec3 light;
	vec3 reflectLight = vec3(0);
	//根据世界空间的法线构建上半球坐标系
	vec3 y = -normalize(n);
	vec3 x,z;
	float f_p_wo_wi = R_lambert / s_pi;
	if((1 - abs(y.x)) > 0.00000001)
	{
		x = vec3(1,0,0);
	}else {
		x = vec3(0,1,0);
	}
	z = normalize(cross(x,y));
	x = normalize(cross(y,z));
	mat3 nornalMatrix = mat3(x,y,z);
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
		reflectLight+= vec3(f_p_wo_wi) * light * dot(n,wi);


	}
	reflectLight /= numSample;//归一化
	return reflectLight;

}


//菲涅尔项，对折射率用一个值表示的介质，给定入射光和法线的夹角以及入射光所在介质和物体介质的相对折射率之比， 计算入射光和法线夹角下反射光反射的比例
float FrenelReflectRatio(float cosTheta_i/*入射光和法线的夹角cos值*/,float eta/*相对折射率:入射光所在的介质的折射率ni  / 折射光所在材质介质的折射率nt */){
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	float cosTheta_t = sqrt(1- pow(sinTheta_i * eta ,2));
	float r_parellel = (cosTheta_i- eta *  cosTheta_t)  / (cosTheta_i + eta * cosTheta_t);
	float r_perpendicular = ( eta * cosTheta_i -cosTheta_t)  / (eta * cosTheta_i + cosTheta_t);



	return (pow(r_parellel,2) + pow(r_perpendicular,2) ) / 2;
}



//完美镜面反射模型，平面是一个完全光滑，入射光会根据法向量以及反射规律完全对称反射出去，反射光的多少遵循frenel定律
vec3 ReflectModelSpecularReflectAndRefract(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){
	vec3 totalLight = vec3(0);
	vec3 wi = reflect(-wo,n);
	vec3 reflectLight = texture(skyTexture,wi).xyz;
	//gama 解码，转线性空间
	reflectLight = pow(reflectLight, vec3(2.4));
	//计算反射光的比例
	float reflectRatio = FrenelReflectRatio(max(dot(wi,n),0),1 / 1.5);
	//折射光的比例
	float refractRatio = 1 - reflectRatio;


	
	//计算反射光的brdf，此时的brdf就为内涅尔项除以反射角和法向量夹角的cos值，此时计算如下:

	float woDotN = max(dot(wi,n),0.000000001);//这项引入作用只是用来表述brdf的形式，所以直接限制最小值防止出现除0错误
	float fr_p_wo_wi = reflectRatio / woDotN;//该模式的brdf
	float wiDotN = woDotN;
	vec3 reflectTotalLight = fr_p_wo_wi * reflectLight * wiDotN;
	totalLight += reflectTotalLight; 
	
	//计算折射光方向
	vec3 wt = refract(-wo,n,1/1.5);
	if(length(wt)!=0)//折射光存在
	{
		float wtDotNegN = max(dot(-n,wt),0.000000001);//这项引入作用只是用来表述brdf的形式，所以直接限制最小值防止出现除0错误
		//计算折射光的btdf
		float ft_p_wt_wi = refractRatio / wtDotNegN;
		//计算从介质折射到wo介质中的btdf
		float ft_p_wi_wt = ft_p_wt_wi * pow(1 / 1.5,2);
		vec3 refractLight = texture(skyTexture,wt).xyz;
		//gama 解码，转线性空间
		refractLight = pow(refractLight, vec3(2.4));
		vec3 refractTotalLight = ft_p_wi_wt * refractLight * wtDotNegN;
		totalLight += refractTotalLight;	
	}

	return totalLight;
}



//这里定义复数的计算法则
//返回一个复数
vec2 Complex(float real)
{
	return vec2(real,0);
}

//复数模长
float ComplexLen(vec2 c)
{
	return sqrt(pow(c.x,2) + pow(c.y,2));
}
vec2 ComplexSqrt(vec2 c)
{
	float sqrtLen = sqrt(ComplexLen(c));
	float halfTheta = atan(c.y,c.x) / 2;
	return vec2(sqrtLen * cos(halfTheta),sqrtLen * sin(halfTheta));
}

// c1 * c2
vec2 ComplexMul(vec2 c1,vec2 c2)
{
	vec2 res;
	res.x = c1.x * c2.x - c1.y*c2.y;
	res.y = c1.x * c2.y + c1.y + c2.x;
	return res;
}

// c1/c2
vec2 ComplexDiv(vec2 c1,vec2 c2)
{
	vec2 res;
	float len2C2 = pow(ComplexLen(c2),2);
	vec2 c1Mulnc2 = ComplexMul(c1,c2 * vec2(1,-1));
	res = c1Mulnc2/len2C2;
	return res;
}

vec2 ComplexAdd(vec2 c1,vec2 c2)
{
	vec2 res;
	res.x = c1.x + c2.x;
	res.y = c1.y + c2.y;
	return res;
}

vec2 ComplexSub(vec2 c1,vec2 c2)
{
	vec2 res;
	res.x = c1.x - c2.x;
	res.y = c1.y - c2.y;
	return res;
}


//菲涅尔项，对折射率用一个复数值表示的介质，一般为金属，给定入射光和法线的夹角以及入射光所在介质和物体介质的相对折射率之比， 计算入射光和法线夹角下反射光反射的比例
float FrenelReflectRatioComplex(float cosTheta_i/*入射光和法线的夹角*/,vec2 etai/*etai.x 表示入射光所在介质的折射率 ，etai.y表示衰减系数k， 该复数表示n+ik */, vec2 etat/*etat.x 表示折射介质的折射率 ，etat.y表示衰减系数k， 该复数表示n+ik */){
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	vec2 etaI_T = ComplexDiv(etai,etat);

	vec2 sinIMulEta = ComplexMul(Complex(sinTheta_i),etaI_T);
	vec2 cosTheta_t_c = ComplexSqrt(Complex(1)- ComplexMul(sinIMulEta,sinIMulEta));
	vec2 cosTheta_i_c = Complex(cosTheta_i);
	vec2 etaMulCosThetat_c = ComplexMul(etaI_T,cosTheta_t_c);
	vec2 r_parellel_c_top =  ComplexSub(cosTheta_i_c,etaMulCosThetat_c);
	vec2 r_parellel_c_bottom =  ComplexAdd(cosTheta_i_c,etaMulCosThetat_c);

	vec2 r_parellel_c = ComplexDiv(r_parellel_c_top,r_parellel_c_bottom);

	vec2 etaMulCosThetai_c = ComplexMul(etaI_T,cosTheta_i_c);
	vec2 r_perpendicular_top =  ComplexSub(etaMulCosThetai_c,cosTheta_t_c);
	vec2 r_perpendicular_bottom =  ComplexAdd(etaMulCosThetai_c,cosTheta_t_c);

	vec2 r_perpendicular_c = ComplexDiv(r_perpendicular_top,r_perpendicular_bottom);


	return (pow(ComplexLen(r_parellel_c),2) + pow(ComplexLen(r_perpendicular_c),2) ) / 2;

}

//金属反射模型，入射光会根据法向量以及反射规律完全对称反射出去，反射光的多少遵循frenel定律
vec3 ReflectModelMetalReflect(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){
	vec3 reflectLight = vec3(0);
	vec3 wi = reflect(-wo,n);
	vec3 light = texture(skyTexture,wi).xyz;
	//gama 解码，转线性空间
	light = pow(light, vec3(2.4));
	//计算反射光的比例
	float reflectRatio = FrenelReflectRatioComplex(max(dot(wi,n),0), vec2(1,0),vec2(0.15,3.9));
	reflectLight = reflectRatio * light;
	return reflectLight;
}

//有粗糙度的使用微表面理论的反射模型
vec3 ReflectModelRoughnessWithMicrofacetTheoryReflect(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){
	uint numSample = 100;

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
	mat3 normalMatrixInverse  = inverse(normalMatrix);
	
	vec3 reflectLight = vec3(0);
	float totalPDF = 0;
	float eta = 1/1.5;
	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//获取二维随机点，为（theta，phi）
		vec2 samplePoint = HaltonSample2D(sampleIndex); 
		//将随机点转化为半球中的方向
		float theta = samplePoint.x * 2* s_pi;
		float fine = 0.5 * s_pi * samplePoint.y;
		vec3 wi;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalize(wi);

		//将wo转到局部空间
		vec3 localWo = normalMatrixInverse * normalize(wo); 

		//获得半向量作为微平面的法向量
		vec3 halfVec = normalize(wi+localWo);
		float nDotWi = clamp(dot(halfVec,wi),0,1);
		
		//计算sparrow模型的 brdf项反射项
		
		float pdf = PDF_Sparrow(halfVec,localWo);//wi的pdf
		float frenel = FrenelReflectRatio(nDotWi,eta) ;//菲涅尔项
		float unmask = UnMaskAndUnShadow2(localWo,wi);//非几何遮蔽和阴影项
		float curbrdf = pdf* frenel * unmask;

		//采样
		wi = normalMatrix * wi;//变换到世界坐标
		wi  = normalize(wi);
		vec3 light = texture(skyTexture,wi).xyz;

		//gama 解码，转线性空间
		light = pow(light, vec3(2.4));

		vec3 curLight = curbrdf * light * nDotWi;
		reflectLight+=  curLight * pdf;//这里不是求的所有光的积分总和，而是求的每个光应该在最终的结果中占据的比例，所以不能除以pdf
		totalPDF +=pdf;


//		//计算折射的BTDF
//		float btdf_p_wo_wi = 1-curbrdf;
//		//计算从材质内部折射出来的光的btdf
//		float btdf_p_wi_wo = Ft_P_Wi_Wo(btdf_p_wo_wi,eta);
//		//计算折射光作为入射光的概率密度
//		float pdf_refract = PDF_Refract(halfVec,localWo,eta);
//		vec3 wi_refract = refract(-localWo,halfVec,eta);
//		wi_refract = normalize(wi);
//		if(length(wi_refract) == 0)
//		{
//			continue;
//		}
//		//将折射入射光变换到世界空间
//		wi_refract = normalMatrix * wi_refract;//变换到世界坐标
//		wi_refract  = normalize(wi_refract);
//		light = texture(skyTexture,wi_refract).xyz;
//		light = pow(light, vec3(2.4));
//		refractTotalLight += pdf_refract * btdf_p_wi_wo * light;
//		totalRefractPDF+= pdf_refract;
	}
	reflectLight /=  totalPDF;//归一化

	return reflectLight;


}


void main(){

	vec3 wo = normalize(viewPosition - inWorldPosition);
	vec3 color;
	//color = ExampleLambertBSDF(wo,inNormal);
	//color += ExampleRoughnessSpecularBRDF(wo,inNormal);
	//color = ExampleSimpleSpecularBRDF(wo,inNormal);
	//color = ExampleSimpleBTDF(wo,inNormal);
	//color = ExampleSparrowBRDT(wo,inNormal);
	//color = ReflectModelLambertDiffuseReflect(wo,inNormal);
	//color = ReflectModelSpecularReflectAndRefract(wo,inNormal);
	//color = ReflectModelMetalReflect(wo,inNormal);
	color = ReflectModelRoughnessWithMicrofacetTheoryReflect(wo,inNormal);
	//gama 解码
	//color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);



	//场景所有对象绘制为白色
	//outColor = vec4(1,1,1,1);


}