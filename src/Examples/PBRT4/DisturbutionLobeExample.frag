#version 450 core


layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 finalColor;



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



void main(){

	vec3 finalPos = inPosition;
	vec3 wo = vec3(1,-0.5,0);
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

		



		



		//pdf = PDF_Sparrow(normal,wo);
		
		vec3 reflv = reflect(-wo,normal);
		reflv= normalize(reflv);
		
		vec3 refrav = refract(-wo,normal,1/1.5);
		refrav= normalize(refrav);

	}else {
		vec3 curVec = normalize(finalPos);
	
		
		float ni = 1.5,no = 1;

		//出射光和xz平面的夹角作为theta1,法向量和xz平面的夹角作为theta2，折射入射光和xz平面的夹角作为theta3
		float theta1 = asin(-wo.y);
		float theta3 = -asin(-curVec.y) + s_pi;
		float thetaSub = theta3 - theta1;


		float theta2 = atan((no * sin(theta1) + ni * sin(theta3))/(no* cos(theta1) + ni* cos(theta3)));
		//thetak + theta1  = theta2,由折射关系，thetak范围为[0,pi/2]
		if(theta2 <0 && theta2 < theta1 - s_pi/2)
		{
			theta2+=s_pi;
		}
		float thetai = theta2 - theta1;

		if(abs(theta2 - theta1) < s_pi/2)
		{
		
			//计算theta2的fine角
			float fine = atan(wo.z,wo.x);
			vec3 wm;
			wm.x = cos(theta2) * cos(fine);
			wm.z = cos(theta2) * sin(fine);
			wm.y = -sin(theta2);
			//normal需要计算
			float dotWmWo = dot(wm,wo);
			float wtPdf = PDF_Refract(wm,wo,1.5/1);

			if(wtPdf < 0.8)
			{
				outColor.z = wtPdf;
			}else {
				outColor.z = 1;
				//outColor.x = 1;
			}
			outColor.z = wtPdf * 0.1;
			//outColor.z = wiPdf;
		
		
		
		
		
		}

	
	}
	
	

	//场景所有对象绘制为白色
	finalColor = vec4(outColor,1);

}