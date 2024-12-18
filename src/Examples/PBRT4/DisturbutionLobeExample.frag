#version 450 core


layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 finalColor;

layout(set = 0,binding = 1,std140) uniform ViewExampleType{
	uint type;//type为0，看分布 ，为1，看sparrow模型的brdf和采样点，为2看分层采样点
};


const float s_pi = 3.141592653;
float roughnessX = 0.2,roughnessY = 0.2;
//GGX分布
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


//基于微表面分布，返回从wo处看，折射入射光wi的分布，这个分布目前看推导是不正确的
float PDF_Refract2(vec3 w,vec3 wo,float eta/*相对折射率: 入射光所在的材质介质的折射率ni  / 出射光所在的介质的折射率no*/){
	//由于

	//获取从wo处看反射光的pdf
	float pdf = PDF_Sparrow(w,wo);
	//基于反射光的分布计算折射入射光的分布 ,反射光的分布和出射光的分布相同
	float wo_pdf = pdf;

	//将折射光作为入射光wi
	vec3 wi = refract(-wo,w,1/eta);
	float cosTheta_i = max(dot(wi,-w),0);
	float cosTheta_o = max(dot(wo,w),0);
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	float sinTheta_o = sqrt(1 - pow(cosTheta_o,2));
	float ratio = sinTheta_i / sinTheta_o;
	 
	if(cosTheta_o == 0)
	{
		return 0 ; 
	}
	//已经知道出射光的分布求折射入射光的分布
	return pdf * pow(eta,2) * cosTheta_i / cosTheta_o;
}


//基于微表面分布，返回从wo处看，折射入射光wi的分布,推导来源来自Microfacet Models for Refraction through Rough Surfaces这篇论文
float PDF_Refract(vec3 w,vec3 wo,float eta/*相对折射率: 入射光所在的材质介质的折射率ni  / 出射光所在的介质的折射率no*/){
	//由于

	//获取从wo处看wm的pdf
	float pdf = PDF2_GGX(w,wo);
	float wmDotWo = max(dot(w,wo),0);
	vec3 wi = refract(-wo,w,1/eta);
	float wiDotWm = dot(wi,w);

	float denom = pow(wiDotWm + wmDotWo / eta,2 );
	if(denom == 0)
	{
		return 0;
	}
	float res = abs(wiDotWm) / denom;
	return res;
}



//菲涅尔项，对折射率用一个值表示的介质，给定入射光和法线的夹角以及入射光所在介质和物体介质的相对折射率之比， 计算入射光和法线夹角下反射光反射的比例
float FrenelReflectRatio(float cosTheta_i/*入射光和法线的夹角cos值*/,float eta/*相对折射率:入射光所在的介质的折射率ni  / 折射光所在材质介质的折射率nt */){
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	float cosTheta_t = sqrt(1- pow(sinTheta_i * eta ,2));
	float r_parellel = (cosTheta_i- eta *  cosTheta_t)  / (cosTheta_i + eta * cosTheta_t);
	float r_perpendicular = ( eta * cosTheta_i -cosTheta_t)  / (eta * cosTheta_i + cosTheta_t);



	return (pow(r_parellel,2) + pow(r_perpendicular,2) ) / 2;
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

//根据一个符合均匀分布的各项范围在-1到1之间的样本，拟合变换得到一个近似微平面反射光的方向和完美反射光之间的theta和fine差值的分布结果，返回值第一项为theta的差值，范围-pi到pi，第二项为fine的差值，范围-pi/2 到pi/2
vec2 FittingInverseCDFSparrowReflect(vec2 daltaXY/*两个分别为-1到1之间的二维参数*/){
	vec2 res;
	//第一项表示和目标向量在theta角的差值，范围-pi到pi
	res.x = asin(pow(daltaXY.x,3)) * 2;
	res.y = asin(pow(daltaXY.y,3));


	return res;
}





void ViewDisturbution(){

	vec3 finalPos = inPosition;
	vec3 wo = vec3(1,-1,0);
	wo = normalize(wo);


	//wo = normalize(w_out);
	vec3 outColor = vec3(0,0,0);
	//wm的概率密度作为红色分量  反射光的概率密度作为绿色分量 折射光的概率密度作为蓝色分量
	if(finalPos.y <=0)
	{
		vec3 normal = normalize(finalPos);
		vec3 curVec = normalize(finalPos);
		float wmPdf = PDF2_GGX(curVec,wo);
		if(wmPdf>0 && dot(curVec,vec3(0,-1,0)) > 0)
		{
			outColor.x += min(wmPdf,1);
		}

		//float pdf = PDF_GGX(normal);

		vec3 halfVec = normalize((curVec + wo)/2);
		//pdf = PDF2_GGX(normal,wo);
		float wiPdf = PDF_Sparrow(halfVec,wo);
		if(wiPdf >0 && dot(halfVec,vec3(0,-1,0)) > 0)
		{
			outColor.y += min(wiPdf,1);
		}

	}else {
		vec3 curVec = normalize(finalPos);
		float ni = 1.5,no = 1;
		//计算半向量
		vec3 wm = -(ni * curVec + no * wo);
		wm = normalize(wm);
		if(length(wm) != 0)
		{
			float wtPdf = PDF_Refract(wm,wo,1.5/1);
			float wmDotN = dot(wm,vec3(0,-1,0));
			outColor.z = wtPdf * 0.2;
		}


	
	}
	
		//场景所有对象绘制为白色
	finalColor = vec4(outColor,1);

	//计算理想反射光和折射光的信息
	vec3 wc = normalize(finalPos);
	vec3 wr = reflect(-wo,vec3(0,-1,0));
	vec3 wt = refract(-wo,vec3(0,-1,0),1/1.5);
	float theta_r = atan(wr.z,wr.x);
	if(theta_r <0)
	{
		theta_r+=2* s_pi;
	}
	float fine_r = acos(-wr.y) ; 

	float theta_t = atan(wt.z,wt.x);
	if(theta_t <0)
	{
		theta_t+=2* s_pi;
	}
	float fine_t = acos(-wt.y); 
	
	float theta_cur = atan(wc.z,wc.x);
	if(theta_cur <0)
	{
		theta_cur+=2* s_pi;
	}
	float fine_cur = acos(-wc.y); 

	float theta_o = atan(wo.z,wo.x);
	if(theta_o <0)
	{
		theta_o+=2* s_pi;
	}
	float fine_o = acos(-wo.y); 



	//画出理想反射光和折射光,入射光以及理想法向量


	//计算是否和反射光很接近
	if(abs(theta_r-theta_cur) < 0.1 && abs(fine_r-fine_cur) < 0.1)
	{
		finalColor= vec4(1,1,1,1);
	}

	//计算是否和折射光很接近
	if(abs(theta_t-theta_cur) < 0.1 && abs(fine_t-fine_cur) < 0.1)
	{
		finalColor= vec4(1,1,1,1);
	}

	//计算是否和出射光很接近
	if(abs(theta_o-theta_cur) < 0.1 && abs(fine_o-fine_cur) < 0.1)
	{
		finalColor= vec4(1,1,1,1);
	}

	//计算是否和理想法向量很接近
	if(abs(fine_cur) < 0.1)
	{
		finalColor= vec4(1,1,1,1);
	}





}



void ViewBRDFAndSample(){

	vec3 finalPos = inPosition;
	vec3 wo = vec3(1,-1,0);
	wo = normalize(wo);
	vec3 wr = reflect(-wo,vec3(0,-1,0));
	float theta_r = atan(wr.z,wr.x);
	if(theta_r <0)
	{
		theta_r+=2* s_pi;
	}
	float fine_r = acos(-wr.y) ; 

	vec3 outColor = vec3(0,0,0);
	//wm的概率密度作为红色分量  反射光的概率密度作为绿色分量 折射光的概率密度作为蓝色分量
	if(finalPos.y <=0)
	{
		vec3 curVec = normalize(finalPos);
		vec3 halfVec = normalize(wo + curVec);

		float wiDotN = dot(curVec,halfVec);
		float frenel = FrenelReflectRatio(wiDotN,1/1.5);
		//vec3 wo = reflect(-curVec,normal);
		float unmask = UnMaskAndUnShadow2(wo,curVec);

		float wiPdf = PDF_Sparrow(halfVec,wo);
		if(wiDotN!=0)
		{
			float fr_p_wo_wi = wiPdf * frenel * unmask / wiDotN;
			finalColor = vec4(fr_p_wo_wi * wiDotN,0,0,1);
		}

		


	//绘制采样点
	for(uint sampleIndex = 0;sampleIndex < 100;sampleIndex++)
	{

		curVec = normalize(curVec);
		//计算当前方向的theta和fine值
		float theta_curV =  atan(curVec.z,curVec.x);
		if(theta_curV <0)
		{
			theta_curV+=2*s_pi;
		}
		float fines_curV = acos(-curVec.y) ; 


		//获取二维随机点，为（theta，phi）
		vec2 samplePoint = HaltonSample2D(sampleIndex); 
		float thetas = 0,fines = 0;
		//均匀分布,计算采样向量的theta和fine值
		thetas = samplePoint.x * 2* s_pi;
		fines = 0.5 * s_pi * samplePoint.y;

		//重要性采样分布，计算采样向量的theta和fine值,使用inverse CDF比较麻烦，不好计算，这里简单使用采样向量和反射向量的余弦值反向获取采样向量
		samplePoint -=0.5;
		samplePoint *=2;//变到-1到1之间
		vec2 deltaThetaFine = FittingInverseCDFSparrowReflect(samplePoint);
				
		
		thetas =  deltaThetaFine.x + theta_r;
		fines = deltaThetaFine.y + fine_r;

		vec3 ws;
		ws.x = cos(thetas)* sin(fines);
		ws.z = sin(thetas)* sin(fines);
		ws.y = -cos(fines);
		ws = normalize(ws);






		//计算是否和某个采样点很接近
		if(abs(thetas-theta_curV) < 0.01 && abs(fines-fines_curV) < 0.01)
		{
			finalColor+=vec4(0,0,1,0);
		}

		
	}



	}else {

		//计算折射
		vec3 curVec = normalize(finalPos);
		float ni = 1.5,no = 1;
		vec3 wm = -(ni * curVec + no * wo);
		wm = normalize(wm);

		float woDotWm = dot(wo,wm);
		float wiDotWm = dot(curVec,-wm);
		float sinWiWm = sqrt(1-pow(wiDotWm,2));
		float sinWoWm = sqrt(1-pow(woDotWm,2));
		float niMulSinWi = ni * sinWiWm;
		float noMulSinWo = no * sinWoWm;
		//计算从平面下方折射出来的比例
		float frenel = FrenelReflectRatio(woDotWm,1/1.5);
		float refractRatio = 1-frenel;
		float wtPdf = 0;

		//finalColor = vec4(0,0,refractRatio,1);
		//return ;
		


		if(length(wm) != 0)
		{
			wtPdf  = PDF_Refract(wm,wo,1.5/1);
			float wmDotN = dot(wm,vec3(0,-1,0));
			outColor.z = wtPdf * 0.2;
		}

		float unmask = UnMaskAndUnShadow2(wo,curVec);


		float ft_p_wo_wi = wtPdf * unmask * refractRatio;
		finalColor = vec4(0,ft_p_wo_wi * 0.1,0,1);
		


		//finalColor = vec4(0,0,0,1);
	
	}








}

void ViewLayeredSample(){
	vec3 finalPos = inPosition; 
	vec3 wc = normalize(finalPos);
	float theta_cur = atan(wc.z,wc.x);
	if(theta_cur <0)
	{
		theta_cur+=2* s_pi;
	}
	float fine_cur = acos(-wc.y); 

	//绘制分层采样的采样点
	uint numStepTheta = 20,numStepFine = 10;
	float deltaTheta = 2 * s_pi / numStepTheta,deltaFine = 0.5 * s_pi / numStepFine;
	finalColor= vec4(0,0,0,1);
	for(uint i = 0;i < numStepTheta;i++)
	{
		for(uint j = 0;j < numStepFine;j++)
		{
			vec2 samplePoint = HaltonSample2D(j* numStepTheta + i);
			float sampleTheta = i * deltaTheta + samplePoint.x * deltaTheta;
			float sampleFine = j * deltaFine + samplePoint.y * deltaFine;

			if(abs(sampleTheta-theta_cur) < 0.01 && abs(sampleFine-fine_cur) < 0.01)
			{
				finalColor= vec4(1,0,1,1);
			}
		
		}
	
	
	}




}

void main(){

	if(type == 0)
	{
		ViewDisturbution();
	
	}else if(type == 1)
	{
		ViewBRDFAndSample();
	}else if(type == 2){
		ViewLayeredSample();
	}
	

}