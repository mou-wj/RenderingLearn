#version 450 core


layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 finalColor;



const float s_pi = 3.141592653;
float roughnessX = 0.2,roughnessY = 0.2;
//GGX�ֲ�
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


//�ڵ������ش�wo���򿴵�΢ƽ��δ���ڵ��ı���
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



//����GGX����һ����wo�йص�΢����ֲ�,�÷ֲ���ʾ�ڹۿ��Ƕ�Ϊwo������£�����w��΢�������
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


//Torrance�CSparrow PDF ����΢����ֲ������������wi�ķֲ�
float PDF_Sparrow(vec3 w,vec3 wo)
{
	float dotWoW = dot(w,wo);
	if(dotWoW == 0){
		return 0;
	}
	return PDF2_GGX(w,wo) / ( 4 * max(0,dotWoW));

}


//����΢����ֲ������ش�wo���������������wi�ķֲ�
float PDF_Refract(vec3 w,vec3 wo,float eta/*���������: ��������ڵĲ��ʽ��ʵ�������ni  / ��������ڵĽ��ʵ�������no*/){
	//����

	//��ȡ��wo����������pdf
	float pdf = PDF_Sparrow(w,wo);
	//���ڷ����ķֲ��������������ķֲ� ,�����ķֲ��ͳ����ķֲ���ͬ
	float wo_pdf = pdf;

	//���������Ϊ�����wi
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
	//�Ѿ�֪�������ķֲ������������ķֲ�
	return pdf * pow(eta,2) * cosTheta_i / cosTheta_o;
}

void ViewDisturbution(){

	vec3 finalPos = inPosition;
	vec3 wo = vec3(1,-0.5,0);
	wo = normalize(wo);
	//wo = normalize(w_out);
	vec3 outColor = vec3(0,0,0);
	//wm�ĸ����ܶ���Ϊ��ɫ����  �����ĸ����ܶ���Ϊ��ɫ���� �����ĸ����ܶ���Ϊ��ɫ����
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

		//������xzƽ��ļн���Ϊtheta1,��������xzƽ��ļн���Ϊtheta2������������xzƽ��ļн���Ϊtheta3
		float theta1 = asin(-wo.y);
		float theta3 = -asin(-curVec.y) + s_pi;
		float thetaSub = theta3 - theta1;


		float theta2 = atan((no * sin(theta1) + ni * sin(theta3))/(no* cos(theta1) + ni* cos(theta3)));
		//thetak + theta1  = theta2,�������ϵ��thetak��ΧΪ[0,pi/2]
		if(theta2 <0 && theta2 < theta1 - s_pi/2)
		{
			theta2+=s_pi;
		}
		float thetai = theta2 - theta1;

		if(abs(theta2 - theta1) < s_pi/2)
		{
		
			//����theta2��fine��
			float fine = atan(wo.z,wo.x);
			vec3 wm;
			wm.x = cos(theta2) * cos(fine);
			wm.z = cos(theta2) * sin(fine);
			wm.y = -sin(theta2);
			//normal��Ҫ����
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
	
	

	//�������ж������Ϊ��ɫ
	finalColor = vec4(outColor,1);


}


//�����������������һ��ֵ��ʾ�Ľ��ʣ����������ͷ��ߵļн��Լ���������ڽ��ʺ�������ʵ����������֮�ȣ� ���������ͷ��߼н��·���ⷴ��ı���
float FrenelReflectRatio(float cosTheta_i/*�����ͷ��ߵļн�cosֵ*/,float eta/*���������:��������ڵĽ��ʵ�������ni  / ��������ڲ��ʽ��ʵ�������nt */){
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	float cosTheta_t = sqrt(1- pow(sinTheta_i * eta ,2));
	float r_parellel = (cosTheta_i- eta *  cosTheta_t)  / (cosTheta_i + eta * cosTheta_t);
	float r_perpendicular = ( eta * cosTheta_i -cosTheta_t)  / (eta * cosTheta_i + cosTheta_t);



	return (pow(r_parellel,2) + pow(r_perpendicular,2) ) / 2;
}

//���ش�wo����δ�����Լ�������Ӱ�еı���
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
		lambdaWo = (sqrt(1 + pow(roughnessWo * tan(fine) ,2)) - 1) / 2;//����wo��lambda��������ֵ
	}

	fine = acos(-wi.y); 
	if(-wi.y != 0)
	{
		theta = atan(wi.z,wi.x);
		float roughnessWi = sqrt(pow(roughnessX * cos(theta),2) + pow(roughnessY * sin(theta),2) );
		lambdaWi = (sqrt(1 + pow(roughnessWi * tan(fine) ,2)) - 1) / 2;//����wi��lambda��������ֵ
	}

	return 1 / (1 + lambdaWi + lambdaWo);
}


//���������
// ����Hammersley ���е�һά�����㣬����ά�ȵ����п���ʹ�ò�ͬ�Ļ�������
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

// ���ɾ��ȷֲ��Ķ�άHalton������
vec2 HaltonSample2D(uint index) {
    return vec2(HammersleySequence(index, 2),HammersleySequence(index, 3)); // ����ѡ�� 2,3
}

//���������ͻ�׼������һ��theta��fine�Ĳ�ֵ�����ػ��ڻ�׼������һ����������
vec2 FittingInverseCDFSparrowReflect(){

	return vec2(0);
}

void ViewBRDFAndSample(){

	vec3 finalPos = inPosition;
	vec3 wo = normalize(vec3(0,-1,-4));
	vec3 outColor = vec3(0,0,0);
	//wm�ĸ����ܶ���Ϊ��ɫ����  �����ĸ����ܶ���Ϊ��ɫ���� �����ĸ����ܶ���Ϊ��ɫ����
	if(finalPos.y <=0)
	{
		vec3 curVec = normalize(finalPos);
		vec3 halfVec = normalize(wo + curVec);

		float wiDotN = dot(curVec,halfVec);
		float frenel = FrenelReflectRatio(wiDotN,1/1.5);
		//vec3 wo = reflect(-curVec,normal);
		float unmask = UnMaskAndUnShadow2(wo,curVec);

		float wiPdf = PDF_Sparrow(halfVec,wo);
		finalColor = vec4(wiPdf * frenel * unmask,0,0,1);
		


	//���Ʋ�����
	for(uint sampleIndex = 0;sampleIndex < 100;sampleIndex++)
	{
		//��ȡ��ά����㣬Ϊ��theta��phi��
		vec2 samplePoint = HaltonSample2D(sampleIndex); 
		float thetas = 0,fines = 0;
		//���ȷֲ�,�������������theta��fineֵ
		thetas = samplePoint.x * 2* s_pi;
		fines = 0.5 * s_pi * samplePoint.y;

		//��Ҫ�Բ����ֲ����������������theta��fineֵ,ʹ��inverse CDF�Ƚ��鷳�����ü��㣬�����ʹ�ò��������ͷ�������������ֵ�����ȡ��������
		samplePoint -=0.5;
		samplePoint *=2;//���-1��1֮��
				
		
		thetas =  samplePoint.x * 2* s_pi;
		fines = atan((roughnessX + roughnessY) * sqrt(samplePoint.y) / sqrt(1 - samplePoint.y));

		vec3 ws;
		ws.x = cos(thetas)* sin(fines);
		ws.z = sin(thetas)* sin(fines);
		ws.y = -cos(fines);
		ws = normalize(ws);

		//�����Ƿ��ĳ��������ܽӽ�
		curVec = normalize(curVec);
		//���㵱ǰ�����theta��fineֵ
		float theta_curV =  atan(curVec.z,curVec.x)  / (2* s_pi) + 0.5;
		float fines_curV = acos(-curVec.y) / (0.5 * s_pi) ; 





		if(abs(thetas-theta_curV) < 0.01 && abs(fines-fines_curV) < 0.01)
		{
			finalColor+=vec4(0,0,1,0);
		}

		
	}



	}else {
		finalColor = vec4(0,0,0,1);
	
	}








}

void main(){
	
	ViewBRDFAndSample();


}