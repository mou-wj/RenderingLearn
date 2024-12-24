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



//frenel 折射 Ni * sin(theta_i) = No * sin(theta_o)  其中Ni No表示各自的折射率，theta_i theta_o表示各自和法线之间的夹角
//两种材质，其入射光Li，出射光Lo满足pow(Ni,2) * Lo = pow(No,2) * Li， 其 BTDF 满足 pow(No,2) * BTDF(p,wi,wo) = pow(Ni,2) * BTDF(p,wo,wi) 



//微表面理论来表示粗糙度

float roughnessX = 0.03,roughnessY = 0.03;//GGX的theta，fine两个维度的粗糙度控制系数
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
	//float denominator = 1;
	float constFactor = s_pi * roughnessX * roughnessY;
	float cosFineWm4 = pow(cos(fine),4);
	float tanFineWm2 = pow(tan(fine),2);
	float cosThetaDivRoughX2 = pow(cos(theta) / roughnessX,2);
	float sinThetaDivRoughY2 = pow(sin(theta)/ roughnessY,2);
	float denominator = constFactor * cosFineWm4  * pow(1 + tanFineWm2 * (cosThetaDivRoughX2 + sinThetaDivRoughY2),2);
	//denominator *= constFactor * cosFineWm4;
	//denominator *= pow(1 + pow(tan(fine),2) * (pow(cos(theta) / roughnessX,2) + pow(sin(theta)/ roughnessY,2)) ,2);
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
	float pdfWm = PDF_GGX(w);
	float unmask = UnMask(wo);

	return unmask * pdfWm * max(0,dot(w,wo)) / cos(fine);

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

//基于微表面分布，返回从wo处看，折射入射光wi的分布,推导来源来自Microfacet Models for Refraction through Rough Surfaces这篇论文
float PDF_Refract(vec3 w,vec3 wo,float eta/*相对折射率: 入射光所在的材质介质的折射率ni  / 出射光所在的介质的折射率no*/){
	//由于

	//获取从wo处看wm的pdf
	float pdf = 0;
	//使用和wo相关的法线分布
	pdf = PDF2_GGX(w,wo);
	//
	//pdf=PDF_GGX(w);
	float wmDotWo = max(dot(w,wo),0);
	vec3 wi = refract(-wo,w,1/eta);
	float wiDotWm = dot(wi,w);

	float denom = pow(wiDotWm + wmDotWo / eta,2 );
	if(denom == 0)
	{
		return 0;
	}
	float res = pdf * abs(wiDotWm) / denom;
	return res;
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


	//计算反射光的贡献
	totalLight+= reflectRatio* reflectLight;

	
	//计算折射光方向
	vec3 wt = refract(-wo,n,1/1.5);
	if(length(wt)!=0)//折射光存在
	{
		vec3 refractSampleLight = texture(skyTexture,wt).xyz;
		//gama 解码，转线性空间
		refractSampleLight = pow(refractSampleLight, vec3(2.4));
		totalLight += refractSampleLight * refractRatio;
		
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

//根据一个符合均匀分布的各项范围在-1到1之间的样本，拟合变换得到一个近似微平面反射光的方向和完美反射光之间的theta和fine差值的分布结果，返回值第一项为theta的差值，范围-pi到pi，第二项为fine的差值，范围-pi/2 到pi/2
vec2 FittingInverseCDFSparrowReflect(vec2 daltaXY/*两个分别为-1到1之间的二维参数*/){
	vec2 res;
	//第一项表示和目标向量在theta角的差值，范围-pi到pi
	res.x = asin(pow(daltaXY.x,3)) * 2;
	res.y = asin(pow(daltaXY.y,3));


	return res;
}


//有粗糙度的使用微表面理论的反射模型
vec3 ReflectModelRoughnessWithMicrofacetTheoryReflect(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){
	uint numSample = 30;

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

	//将反射向量变换到法向量的本地坐标系中
	vec3 wr = reflect(-wo,n);
	wr = normalize(normalMatrixInverse * wr);
	//计算wr在本地坐标系中的theta值和fine值
	float theta_r = atan(wr.z,wr.x);
	if(theta_r <0)
	{
		theta_r+=2* s_pi;
	}
	float fine_r = acos(-wr.y) ; 


	
	vec3 reflectLight = vec3(0);
	float totalWeight = 0;
	float eta = 1/1.5;
	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//获取二维随机点，为（theta，phi）
		vec2 samplePoint = HaltonSample2D(sampleIndex); 


		//重要性采样分布，计算采样向量的theta和fine值,使用inverse CDF比较麻烦，不好计算，这里简单使用采样向量和反射向量的余弦值反向获取采样向量
		samplePoint -=0.5;
		samplePoint *=2;//变到-1到1之间
		vec2 deltaThetaFine = FittingInverseCDFSparrowReflect(samplePoint);
			
		//根据反射向量的theta和fine值以及变换后的样本点和反射向量theta和fine的差值获得样本向量的theta和fine值
		float theta = deltaThetaFine.x + theta_r;
		float fine = deltaThetaFine.y + fine_r;

		vec3 wi;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalize(wi);

		//将wo转到局部空间
		vec3 localWo = normalize( normalMatrixInverse * normalize(wo)); 
		
		//获得半向量作为微平面的法向量
		vec3 halfVec = normalize(wi+localWo);
		float nDotWi = clamp(dot(halfVec,wi),0,1);
		if(nDotWi ==0)//当前为无效样本
		{
			continue;
		}


		
		//计算sparrow模型的 brdf项反射项
		
		float pdf = PDF_Sparrow(halfVec,localWo);//wi的pdf
		if(pdf == 0)
		{
			continue;
		}


		float frenel = FrenelReflectRatio(nDotWi,eta) ;//菲涅尔项
		float unmask = UnMaskAndUnShadow2(localWo,wi);//非几何遮蔽和阴影项
		float curbrdf = pdf* frenel * unmask / nDotWi;

		//采样
		wi = normalMatrix * wi;//变换到世界坐标
		wi  = normalize(wi);
		vec3 light = texture(skyTexture,wi).xyz;


		//gama 解码，转线性空间
		light = pow(light, vec3(2.4));
		float curWeight = curbrdf * nDotWi;//权重,由于sparrow模型的brdf中含有pdf项，所以不能再除以pdf，通过重要性采样的的方式计算出射光，再除以pdf就重复处理了pdf，也就失去pdf的意义
		vec3 curLight = curWeight * light;
		reflectLight+=  curLight;
		totalWeight += curWeight;
	}
	reflectLight /=  totalWeight;//归一化
	return reflectLight;


}

//有粗糙度的使用微表面理论的折射模型
vec3 ReflectModelRoughnessWithMrcrofacetTheoryRefract(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){
	wo = normalize(wo);
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


	vec3 refractLight = vec3(0);
	float totalWeight = 0;

	uint numStepTheta = 30,numStepFine = 10;
	float deltaTheta = 2*s_pi / numStepTheta,deltaFine = 0.5*s_pi / numStepFine;
	float ni = 1.5,no = 1;


	//计算理想折射光的theta和fine
	vec3 wt = refract(-wo,n,no/ni);
	wt = normalize(normalMatrixInverse * wt);

	//出射向量转到局部空间
	wo = normalize(normalMatrixInverse * wo);

	

	//计算wt在本地坐标系中的theta值和fine值
	float theta_t = atan(wt.z,wt.x);
	if(theta_t <0)
	{
		theta_t+=2* s_pi;
	}
	float fine_t = acos(-wt.y) ; 

	uint numSample = 20;
	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//获取二维随机点，为（theta，phi）
		vec2 samplePoint = HaltonSample2D(sampleIndex); 


		//重要性采样分布，计算采样向量的theta和fine值,使用inverse CDF比较麻烦，不好计算，这里简单使用采样向量和反射向量的余弦值反向获取采样向量
		samplePoint -=0.5;
		samplePoint *=2;//变到-1到1之间
		vec2 deltaThetaFine = FittingInverseCDFSparrowReflect(samplePoint);
			
		//根据折射向量的theta和fine值以及变换后的样本点和反射向量theta和fine的差值获得样本向量的theta和fine值
		float theta = deltaThetaFine.x + theta_t;
		float fine = deltaThetaFine.y + fine_t;

		vec3 wi;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalize(wi);

		//计算wm
		vec3 wm = -(ni * wi + no * wo);
		wm = normalize(wm);
		float wtPdf = 0;

		if(length(wm) != 0)
		{
			wtPdf  = PDF_Refract(wm,wo,ni/no);

			float wiDotWm = dot(wi,-wm);

			if(wiDotWm != 0 && wtPdf>0)
			{
				float unmask = UnMaskAndUnShadow2(wo,wi);
				float woDotWm = dot(wo,wm);
				//计算折射比例
				float frenel = FrenelReflectRatio(woDotWm,no/ni);
				float refractRatio = 1-frenel;
				float ft_p_wo_wi = wtPdf * unmask * refractRatio / wiDotWm;
				//采样
				wi = normalMatrix * wi;//变换到世界坐标
				wi  = normalize(wi);
				vec3 light = texture(skyTexture,wi).xyz;
				//gama 解码，转线性空间
				light = pow(light, vec3(2.4));
				float curWeight = ft_p_wo_wi * wiDotWm;
				//vec3 cl = curWeight * light ;
				refractLight += light * curWeight;
				totalWeight += curWeight;
				
			}
	

		}

	}



	
	refractLight /=  totalWeight;//归一化

	return refractLight;


}

//有粗糙度的使用微表面理论的反射加折射模型
vec3 ReflectModelRoughnessWithMrcrofacetTheoryReflectAndRefract(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){
	wo = normalize(wo);
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

	vec3 totalLight = vec3(0);
	float totalWeight = 0;
	float ni = 1.5,no = 1;

	uint numSample = 30;

	//计算理想反射和折射向量

	//将反射向量变换到法向量的本地坐标系中
	vec3 wr = reflect(-wo,n);
	wr = normalize(normalMatrixInverse * wr);
	//计算wr在本地坐标系中的theta值和fine值
	float theta_r = atan(wr.z,wr.x);
	if(theta_r <0)
	{
		theta_r+=2* s_pi;
	}
	float fine_r = acos(-wr.y) ; 


	//计算理想折射光的theta和fine
	vec3 wt = refract(-wo,n,no/ni);
	wt = normalize(normalMatrixInverse * wt);

	//计算wt在本地坐标系中的theta值和fine值
	float theta_t = atan(wt.z,wt.x);
	if(theta_t <0)
	{
		theta_t+=2* s_pi;
	}
	float fine_t = acos(-wt.y) ; 

	//出射向量转到局部空间
	wo = normalize(normalMatrixInverse * wo);
	float refractTotalWeight = 0,reflectTotalWeight = 0;

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//获取二维随机点，为（theta，phi）
		vec2 samplePoint = HaltonSample2D(sampleIndex); 


		//重要性采样分布，计算采样向量的theta和fine值,使用inverse CDF比较麻烦，不好计算，这里简单使用采样向量和反射向量的余弦值反向获取采样向量
		samplePoint -=0.5;
		samplePoint *=2;//变到-1到1之间
		vec2 deltaThetaFine = FittingInverseCDFSparrowReflect(samplePoint);
			
		//计算反射
		if(true){
			float sampleTheta = theta_r+deltaThetaFine.x;
			float sampleFine = fine_r+deltaThetaFine.y;
			vec3 wi;
			wi.x = cos(sampleTheta)* sin(sampleFine);
			wi.z = sin(sampleTheta)* sin(sampleFine);
			wi.y = -cos(sampleFine);
			wi = normalize(wi);

		
			//获得半向量作为微平面的法向量
			vec3 halfVec = normalize(wi+wo);
			float nDotWir = clamp(dot(halfVec,wi),0,1);
		
			//计算sparrow模型的 brdf项反射项
		
			float pdf = PDF_Sparrow(halfVec,wo);//wi的pdf
			if(pdf != 0 && nDotWir!=0)
			{
				float frenel = FrenelReflectRatio(nDotWir,no / ni) ;//菲涅尔项
				float unmask = UnMaskAndUnShadow2(wo,wi);//非几何遮蔽和阴影项
				float curbrdf = pdf* frenel * unmask / nDotWir;

				//采样
				wi = normalMatrix * wi;//变换到世界坐标
				wi  = normalize(wi);
				vec3 light = texture(skyTexture,wi).xyz;


				//gama 解码，转线性空间
				light = pow(light, vec3(2.4));
				float curWeight = curbrdf * nDotWir;//权重,由于sparrow模型的brdf中含有pdf项，所以不能再除以pdf，通过重要性采样的的方式计算出射光，再除以pdf就重复处理了pdf，也就失去pdf的意义
				vec3 curLight = light * curWeight;
				totalWeight += curWeight;
				totalLight+= curLight;
				reflectTotalWeight+=curWeight;
			}		
		
		}

		//计算折射
		if(true){

			float sampleTheta = theta_t+deltaThetaFine.x;
			float sampleFine = fine_t+deltaThetaFine.y;

			vec3 wi;
			wi.x = cos(sampleTheta)* sin(sampleFine);
			wi.z = sin(sampleTheta)* sin(sampleFine);
			wi.y = -cos(sampleFine);
			wi = normalize(wi);

			//计算wm
			vec3 wm = -(ni * wi + no * wo);
			wm = normalize(wm);
			float wtPdf = 0;

			if(length(wm) != 0)
			{
				wtPdf  = PDF_Refract(wm,wo,ni/no);

				float wiDotWm = dot(wi,-wm);

				if(wiDotWm != 0 && wtPdf>0)
				{
					float unmask = UnMaskAndUnShadow2(wo,wi);
					float woDotWm = dot(wo,wm);
					//计算折射比例
					float frenel = FrenelReflectRatio(woDotWm,no/ni);
					float refractRatio = 1-frenel;
					float ft_p_wo_wi = wtPdf * unmask * refractRatio / wiDotWm;
					//采样
					wi = normalMatrix * wi;//变换到世界坐标
					wi  = normalize(wi);
					vec3 light = texture(skyTexture,wi).xyz;
					//gama 解码，转线性空间
					light = pow(light, vec3(2.4));
					float curWeight = ft_p_wo_wi * wiDotWm;
					//vec3 cl = curWeight * light ;
					totalLight += light * curWeight;
					totalWeight += curWeight;
					refractTotalWeight +=curWeight;
				}
	

			}

		}	

	}


	totalLight/=totalWeight;//归一化

	//vec3 res = (reflectLight + refractLight)/(reflectTotalWeight + refractTotalWeight);
	return totalLight; 


}

//材质的brdf通过测量得到的反射模型  
vec3 ReflectModelMesuredBRDF(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){

	vec3 res;
	//to do  目前如果要完成mesured bsdf的示例，需要了解.bsdf这种文件格式以及其中数据表达的意思，需要一定时间，所以这一节先暂时跳过
	return res;

}



//头发的反射模型  

float I0(float x) {
    float val = 0;
    float x2i = 1;
    float ifact = 1;
    int i4 = 1;
    // I0(x) \approx Sum_i x^(2i) / (4^i (i!)^2)
    for (int i = 0; i < 10; ++i) {
        if (i > 1)
            ifact *= i;
        val += x2i / (i4 * pow(ifact,2));
        x2i *= x * x;
        i4 *= 4;
    }
    return val;
}
float v[4];
void CaculateVRoughness(){
	

}

//Mp(wo,wi)  ,本地坐标系，
float HairLongitudinalScatteringFunction(vec3 wo,vec3 wi,float v/*粗糙度*/){
	vec3 alongD = vec3(0,0,1);
	vec3 curNormal = normalize(inNormal);
	float sinTheta_o = dot(wo,alongD);
	float cosTheta_o = sqrt(1 - pow(sinTheta_o,2));
	float sinTheta_i = dot(wi,alongD);
	float cosTheta_i = sqrt(1 - pow(sinTheta_i,2));
	float denomination = 2 * v * sinh(1 / v);
	float fac1 = exp(- (sinTheta_i * sinTheta_o) / v);
	float fac2 = I0(cosTheta_i * cosTheta_o / v);

	float res = fac1 * fac2 / denomination;


	return res;
} 



//由于现在条件限制，找不到头发丝用圆柱建模的模型，能找到的基本都是用条带表示，然后加上体渲染来进行头发的显示，所以在这里简单起见，直接用一个细圆柱表示一根头发然后来验证
//这个圆柱的中心线的方向向量为(0,-1,0),故法平面就为xz平面 圆柱直径为0.1
vec3 ReflectModelForHair(vec3 wo/*世界空间中的出射向量*/,vec3 n/*世界空间中的法向量*/){
	//头发模型的6个参数
	float h=0;//出射向量wo在圆柱的垂直直径上的投影点和半径的比例，范围为-1，1
	float eta = 0;//头发的折射率
	float sigma_a = 0;//头发的吸收系数
	float beta_m = 0;//和圆柱界面夹角相关的粗糙度，范围0，1
	float beta_n = 0;//和圆柱中心线夹角相关的粗糙度，范围0，1
	float alpha = 0;//头发最外层鞘和圆柱边缘母线的夹角，一般为2度

	float v = 0.02;//粗糙度,这里直接定义，实际上该值应该通过beta_m计算得到
	vec3 alongD = vec3(0,0,1);//世界空间中的头发方向向量

	//查看是否n和alongD垂直
	float nDotAlongD = dot(alongD,n);
	vec3 res;
	n = normalize(n);
	wo = normalize(wo);


	//根据alongD和n构建本地坐标系，其中alongD作为z，n作为-y
	vec3 y = -normalize(n);
	vec3 z = normalize(alongD);
	vec3 x = normalize(cross(y,z));
	mat3 localMatrix  = mat3(x,y,z);
	mat3 inverseMatrix = inverse(localMatrix);
	//获取反射光向量
	vec3 wri = reflect(-wo,n);
	//将wri和wo转换到本地坐标系
	wri = normalize(inverseMatrix * wri);
	wo = normalize(inverseMatrix * wo);

	float longtitudeScaterFactor  = HairLongitudinalScatteringFunction(wo,wri,v);





	res.x = longtitudeScaterFactor;





	





	
	return res;

}


void main(){

	vec3 wo = normalize(viewPosition - inWorldPosition);
	vec3 color;
	//color = ReflectModelLambertDiffuseReflect(wo,inNormal);
	//color = ReflectModelSpecularReflectAndRefract(wo,inNormal);
	//color = ReflectModelMetalReflect(wo,inNormal);
	//color = ReflectModelRoughnessWithMicrofacetTheoryReflect(wo,inNormal);
	//color = ReflectModelRoughnessWithMrcrofacetTheoryRefract(wo,inNormal);
	
	//color = ReflectModelRoughnessWithMrcrofacetTheoryReflectAndRefract(wo,inNormal);
	color = ReflectModelForHair(wo,inNormal);
	
	//gama 解码
	//color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);



	//场景所有对象绘制为白色
	//outColor = vec4(1,1,1,1);


}