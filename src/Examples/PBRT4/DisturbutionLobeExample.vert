#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 in3;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outColor;
layout(set = 0,binding = 0,std140) uniform SimpleSceenExampleBuffer{
	mat4 world;
	mat4 view;
	mat4 proj;
	vec3 w_out;
};
const float s_pi = 3.141592653;
float roughnessX = 0.2,roughnessY = 0.4;
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


//基于微表面分布，返回从wo处看，折射入射光wi的分布
float PDF_Refract(vec3 w,vec3 wo,float eta/*相对折射率: 入射光所在的材质介质的折射率ni  / 出射光所在的介质的折射率no*/){
	//由于

	//获取从wo处看反射光的pdf
	float pdf = PDF_Sparrow(w,wo);
	//基于反射光的分布计算折射入射光的分布 ,反射光的分布和出射光的分布相同
	float wo_pdf = pdf;

	//将折射光作为入射光wi
	vec3 wi = refract(-wo,w,1/eta);
	float cosTheta_i = max(dot(wi,-w),0);
	float cosTheta_o = max(dot(wo,w),0);

	if(cosTheta_o == 0)
	{
		return 0 ; 
	}
	//已经知道出射光的分布求折射入射光的分布
	return pdf * pow(eta,2) * cosTheta_i / cosTheta_o;
}


void main(){
	vec3 finalPos = inPosition;
	vec3 wo = vec3(1,-1,0);
	wo = normalize(wo);
	//wo = normalize(w_out);
	outColor = vec3(0,0,0);
	//wm的概率密度作为红色分量  反射光的概率密度作为绿色分量 折射光的概率密度作为蓝色分量
	if(finalPos.y <=0)
	{
		vec3 normal = normalize(finalPos);
		vec3 curVec = normalize(finalPos);
		float wmPdf = PDF2_GGX(curVec,wo);
		if(wmPdf>0)
		{
			outColor.x += min(wmPdf,1);
		}

		//float pdf = PDF_GGX(normal);

		vec3 halfVec = normalize((curVec + wo)/2);
		//pdf = PDF2_GGX(normal,wo);
		float wiPdf = PDF_Sparrow(halfVec,wo);
		if(wiPdf >0)
		{
			outColor.y += min(wiPdf,1);
		}

		
		//normal需要计算
		float wtPdf = PDF_Refract(normal,wo,1.5/1);

		//pdf = PDF_Sparrow(normal,wo);
		
		vec3 reflv = reflect(-wo,normal);
		reflv= normalize(reflv);
		
		vec3 refrav = refract(-wo,normal,1/1.5);
		refrav= normalize(refrav);

	}
	

	gl_Position = proj * view * world *vec4(finalPos,1.0f);
	

}