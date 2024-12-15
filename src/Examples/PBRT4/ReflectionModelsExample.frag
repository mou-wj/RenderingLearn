#version 450 core

const float s_pi = 3.141592653;

layout(location = 0) in vec3 inNormal;//��������ռ��еķ���
layout(location = 1) in vec3 inWorldPosition;//��������ռ��е�λ��


layout(location = 0) out vec4 outColor;


layout(set = 0,binding = 1) uniform samplerCube skyTexture;
layout(set = 0,binding = 2) uniform SceenInfo{//������Ϣ
	vec3 viewPosition;//�������������ϵ�е�λ��

};


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



//BSDF ΪBRDF��BTDF��ͳ��


//�򵥷���ģ�� Diffuse Reflection  Lambert model ������
//Diffuse Reflection


float s_LamberRefectionFactor = 0.2;//lambertģ�ͷ�����
float s_LambertxFactor  = s_LamberRefectionFactor / s_pi;


//���س����Ϊwo����£������ĸ����ܶ�
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
	//��������ռ�ķ��߹����ϰ�������ϵ
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
		//��ȡ��ά����㣬Ϊ��theta��phi��
		samplePoint = HaltonSample2D(sampleIndex); 
		//�������ת��Ϊ�����еķ���
		float theta = samplePoint.x * 2* s_pi;
		float fine = 0.5 * s_pi * samplePoint.y;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = nornalMatrix * wi;//�任����������
		wi  = normalize(wi);
		light = texture(skyTexture,wi).xyz;
		//gama ���룬ת���Կռ�
		light = pow(light, vec3(2.4));
		pdf = PDFLambert(wi,wo);
		reflectLight+= BSDFLambert(wo,wi) * light * dot(n,wi) * pdf;
		totalPDF+=pdf;

	}
	reflectLight /= totalPDF;//��һ��
	return reflectLight;

}


//frenel ���� Ni * sin(theta_i) = No * sin(theta_o)  ����Ni No��ʾ���Ե������ʣ�theta_i theta_o��ʾ���Ժͷ���֮��ļн�
//���ֲ��ʣ��������Li�������Lo����pow(Ni,2) * Lo = pow(No,2) * Li�� �� BTDF ���� pow(No,2) * BTDF(p,wi,wo) = pow(Ni,2) * BTDF(p,wo,wi) 


//�򵥾��淴��ģ��
const float specularReflectionFactor = 0.8;

vec3 BRDFSimpleSpecular(vec3 wo,vec3 wi)
{
	vec3 res;
	res.x = specularReflectionFactor;
	res.y = specularReflectionFactor;
	res.z = specularReflectionFactor;
	return res;
	
}

const float speclarFactor = 30;//���Ƹ߹�����

float PDFSimpleSpecular(float NdotH/*���ߺͰ��֮���cosֵ*/){
	float res;
	//��blinn-phong���淴��pdf
	float fac = pow(NdotH,speclarFactor);
	res = (speclarFactor + 2)  *  fac / (2 * s_pi);
	return res;
}

//��һ���ֲڶȵľ��淴��
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
		//��ȡ��ά����㣬Ϊ��theta��phi��
		samplePoint = HaltonSample2D(sampleIndex); 
		//�������ת��Ϊ�����еķ���
		float theta = samplePoint.x * 2* s_pi;
		float fine = 0.5 * s_pi * samplePoint.y;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalMatrix * wi;//�任����������
		wi  = normalize(wi);
		light = texture(skyTexture,wi).xyz;
		//����half������
		halfVec = normalize(wi+wo);
		nDotH = clamp(dot(halfVec,n),0,1);
		pdf = PDFSimpleSpecular(nDotH);


		//gama ���룬ת���Կռ�
		light = pow(light, vec3(2.4));
		if(pdf > 0.0001)
		{
			nDotWi = clamp(dot(n,wi),0,1);
			vec3 curLight = BRDFSimpleSpecular(wo,wi) * light * nDotWi;
			reflectLight+=  curLight * pdf;//���ﲻ��������й�Ļ����ܺͣ��������ÿ����Ӧ�������յĽ����ռ�ݵı��������Բ��ܳ���pdf
			totalPDF +=pdf;
		
		}
	}
	//reflectLight /= numSample;
	reflectLight /=  totalPDF;//��һ��
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


//΢������������ʾ�ֲڶ�

float roughnessX = 0.3,roughnessY = 0.3;//GGX��theta��fine����ά�ȵĴֲڶȿ���ϵ��
//GGX ΢����ֲ������ذ�Բ�ڳ���w�ĵ�΢����ı��ʣ������w���ھֲ��ռ�,�÷ֲ���wo�޹�
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

//���ش�wo����δ�����Լ�������Ӱ�еı���
float UnMaskAndUnShadow(vec3 wo, vec3 wi){

	return UnMask(wo) * UnMask(wi);
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
float PDF_Refract(vec3 w,vec3 wo,float eta/*���������: ����Ĳ��ʽ��ʵ�������ni  / ��������ڵĽ��ʵ�������no*/){
	//����
	vec3 wi = reflect(-wo,w);
	wi = normalize(wi);
	//��ȡ��wo����������pdf
	float pdf = PDF_Sparrow(w,wo);
	//���ڷ����ķֲ��������������ķֲ�   dwi/dwt = dwo/dwt
	float cosTheta_i = max(dot(wi,w),0);
	vec3 wt = refract(-wo,w,eta);
	float cosTheta_t = max(dot(wt,-w),0);
	if(cosTheta_i == 0)
	{
		return 0 ; 
	}
	float cosTheta_o = cosTheta_i;//������cosֵ���ڷ�����cosֵ
	cosTheta_i = cosTheta_t;//�������Ϊ������cosֵ
	return pdf * pow(eta,2) * cosTheta_i / cosTheta_o;
}


//��������  ���㷴���ı���
float Frenel_Reflect(float cosTheta_i/*�����ͷ��ߵļн�cosֵ*/,float eta/*���������: ����Ĳ��ʽ��ʵ������� / ��������ڵĽ��ʵ������� */){
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	float cosTheta_t = sqrt(1-pow(sinTheta_i / eta ,2));
	float r_parellel = (eta *  cosTheta_i- cosTheta_t)  / (eta * cosTheta_i + cosTheta_t);
	float r_perpendicular = (cosTheta_i - eta * cosTheta_t)  / (cosTheta_i + eta * cosTheta_t);



	return (pow(r_parellel,2) + pow(r_perpendicular,2) ) / 2;
}

//��������  �򵥼��������ı��� ��ֱ��ʹ��1��ȥ����������ķ������
float Refract_Simple_Frenel(float cosTheta_i/*�����ͷ��ߵļн�cosֵ*/,float eta/*���������: ����Ĳ��ʽ��ʵ������� / ��������ڵĽ��ʵ������� */){
	return 1 - Frenel_Reflect(cosTheta_i,eta);
}

float Ft_P_Wo_Wi(float fr_P_Wo_Wi){
	return 1-fr_P_Wo_Wi;
}

//��������⵽������btdfϵ�����������⵽������btdfϵ��
//pow(No,2) * BTDF(p,wi,wo) = pow(Ni,2) * BTDF(p,wo,wi) 
float Ft_P_Wi_Wo(float ft_P_Wo_Wi/*����⵽����*/,float eta/*���������: ����Ĳ��ʽ��ʵ�������no / ��������ڵĽ��ʵ������� ni*/)
{
	return pow(1/eta,2) * ft_P_Wo_Wi;
}

vec2 ComplexMultiply(vec2 complex1,vec2 complex2)
{
	return vec2(complex1.x * complex2.x - complex1.y * complex2.y , complex1.x * complex2.y + complex1.y * complex2.x );
}

//����������ʽ��frenel��
float Frenel_Reflect_Complex(float cosTheta_i/*�����ͷ��ߵļн�*/,vec2 eta/*eta.x ��ʾ���������: ����Ĳ��ʽ��ʵ������� / ��������ڵĽ��ʵ������� ��eta.y��ʾ˥��ϵ��k�� �ø�����ʾn+ik */){
	float n2Ak2 = pow(length(eta),2);
	float cosTheta_i2 = pow(cosTheta_i,2);
	float r_parellel = (n2Ak2 - 2 *eta.x * cosTheta_i + cosTheta_i2) / (n2Ak2 + 2 *eta.x * cosTheta_i + cosTheta_i2);
	float r_perpendicular = (n2Ak2 * cosTheta_i - 2 *eta.x * cosTheta_i + cosTheta_i2) / (n2Ak2 * cosTheta_i+ 2 *eta.x * cosTheta_i + cosTheta_i2);
    return (pow(r_parellel,2) + pow(r_perpendicular,2)) / 2;

}

//Sparrow ģ�͵�BRDFʾ��
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
		//��ȡ��ά����㣬Ϊ��theta��phi��
		samplePoint = HaltonSample2D(sampleIndex); 
		//�������ת��Ϊ�����еķ���
		float theta = samplePoint.x * 2* s_pi;
		float fine = 0.5 * s_pi * samplePoint.y;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalize(wi);
		//��woת���ֲ��ռ�
		vec3 localWo = normalMatrixInverse * normalize(wo); 
		//��ð�����
		halfVec = normalize(wi+localWo);

		//����sparrowģ�͵� brdf�����
		
		pdf = PDF_Sparrow(halfVec,localWo);//wi��pdf
		float cosXXX = max(dot(halfVec,localWo),0);
		float frenel = Frenel_Reflect(max(dot(wi,vec3(0,-1,0)),0),eta) ;//��������
		float unmask = UnMaskAndUnShadow2(localWo,wi);//�Ǽ����ڱκ���Ӱ��
		float curbrdf = pdf* frenel * unmask;

		//����
		wi = normalMatrix * wi;//�任����������
		wi  = normalize(wi);
		light = texture(skyTexture,wi).xyz;

		//gama ���룬ת���Կռ�
		light = pow(light, vec3(2.4));
		nDotWi = clamp(dot(n,wi),0,1);
		vec3 curLight = curbrdf * light * nDotWi;
		reflectLight+=  curLight * pdf;//���ﲻ��������й�Ļ����ܺͣ��������ÿ����Ӧ�������յĽ����ռ�ݵı��������Բ��ܳ���pdf
		totalPDF +=pdf;


		//���������BTDF
		float btdf_p_wo_wi = 1-curbrdf;
		//����Ӳ����ڲ���������Ĺ��btdf
		float btdf_p_wi_wo = Ft_P_Wi_Wo(btdf_p_wo_wi,eta);
		//�����������Ϊ�����ĸ����ܶ�
		float pdf_refract = PDF_Refract(halfVec,localWo,eta);
		vec3 wi_refract = refract(-localWo,halfVec,eta);
		wi_refract = normalize(wi);
		if(length(wi_refract) == 0)
		{
			continue;
		}
		//�����������任������ռ�
		wi_refract = normalMatrix * wi_refract;//�任����������
		wi_refract  = normalize(wi_refract);
		light = texture(skyTexture,wi_refract).xyz;
		light = pow(light, vec3(2.4));
		refractTotalLight += pdf_refract * btdf_p_wi_wo * light;
		totalRefractPDF+= pdf_refract;
	}
	//reflectLight /= numSample;
	reflectLight /=  totalPDF;//��һ��
	reflectLight *= (totalPDF) /(totalPDF + totalRefractPDF);
	refractTotalLight /= totalRefractPDF;//��һ��
	refractTotalLight *= (totalRefractPDF) /(totalPDF + totalRefractPDF);

	//ֱ�Ӳ��������
	//vec3 cur_ss = refract(-wo,n,1.2/1);
	//cur_ss  = normalize(cur_ss);
	//light = texture(skyTexture,cur_ss).xyz;
	//light = pow(light, vec3(2.4));

	vec3 resLight = reflectLight + refractTotalLight;
	return resLight;



}

//lambert������ģ�ͣ�����ⱻ���ȷ��䵽��Բ�ĸ���������ÿ�������ķ�����f_p_wo_wi = R / PI,RΪ����ϵ������ʾ����Ĺ��ж��ٻᱻ�����ȥ
float R_lambert = 0.8;//����lambert�ķ���ϵ��
//��ȡ�ڳ����wo����ƽ�淨��Ϊn������£�lambertģ�ͷ��صĳ����Ĵ�С
vec3 ReflectModelLambertDiffuseReflect(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){
	
	uint numSample = 100;
	vec2 samplePoint;
	vec3 wi;
	vec3 light;
	vec3 reflectLight = vec3(0);
	//��������ռ�ķ��߹����ϰ�������ϵ
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
		//��ȡ��ά����㣬Ϊ��theta��phi��
		samplePoint = HaltonSample2D(sampleIndex); 
		//�������ת��Ϊ�����еķ���
		float theta = samplePoint.x * 2* s_pi;
		float fine = 0.5 * s_pi * samplePoint.y;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = nornalMatrix * wi;//�任����������
		wi  = normalize(wi);
		light = texture(skyTexture,wi).xyz;
		//gama ���룬ת���Կռ�
		light = pow(light, vec3(2.4));
		reflectLight+= vec3(f_p_wo_wi) * light * dot(n,wi);


	}
	reflectLight /= numSample;//��һ��
	return reflectLight;

}


//�����������������һ��ֵ��ʾ�Ľ��ʣ����������ͷ��ߵļн��Լ���������ڽ��ʺ�������ʵ����������֮�ȣ� ���������ͷ��߼н��·���ⷴ��ı���
float FrenelReflectRatio(float cosTheta_i/*�����ͷ��ߵļн�cosֵ*/,float eta/*���������:��������ڵĽ��ʵ�������ni  / ��������ڲ��ʽ��ʵ�������nt */){
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	float cosTheta_t = sqrt(1- pow(sinTheta_i * eta ,2));
	float r_parellel = (cosTheta_i- eta *  cosTheta_t)  / (cosTheta_i + eta * cosTheta_t);
	float r_perpendicular = ( eta * cosTheta_i -cosTheta_t)  / (eta * cosTheta_i + cosTheta_t);



	return (pow(r_parellel,2) + pow(r_perpendicular,2) ) / 2;
}



//�������淴��ģ�ͣ�ƽ����һ����ȫ�⻬����������ݷ������Լ����������ȫ�ԳƷ����ȥ�������Ķ�����ѭfrenel����
vec3 ReflectModelSpecularReflectAndRefract(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){
	vec3 totalLight = vec3(0);
	vec3 wi = reflect(-wo,n);
	vec3 reflectLight = texture(skyTexture,wi).xyz;
	//gama ���룬ת���Կռ�
	reflectLight = pow(reflectLight, vec3(2.4));
	//���㷴���ı���
	float reflectRatio = FrenelReflectRatio(max(dot(wi,n),0),1 / 1.5);
	//�����ı���
	float refractRatio = 1 - reflectRatio;


	
	//���㷴����brdf����ʱ��brdf��Ϊ����������Է���Ǻͷ������нǵ�cosֵ����ʱ��������:

	float woDotN = max(dot(wi,n),0.000000001);//������������ֻ����������brdf����ʽ������ֱ��������Сֵ��ֹ���ֳ�0����
	float fr_p_wo_wi = reflectRatio / woDotN;//��ģʽ��brdf
	float wiDotN = woDotN;
	vec3 reflectTotalLight = fr_p_wo_wi * reflectLight * wiDotN;
	totalLight += reflectTotalLight; 
	
	//��������ⷽ��
	vec3 wt = refract(-wo,n,1/1.5);
	if(length(wt)!=0)//��������
	{
		float wtDotNegN = max(dot(-n,wt),0.000000001);//������������ֻ����������brdf����ʽ������ֱ��������Сֵ��ֹ���ֳ�0����
		//����������btdf
		float ft_p_wt_wi = refractRatio / wtDotNegN;
		//����ӽ������䵽wo�����е�btdf
		float ft_p_wi_wt = ft_p_wt_wi * pow(1 / 1.5,2);
		vec3 refractLight = texture(skyTexture,wt).xyz;
		//gama ���룬ת���Կռ�
		refractLight = pow(refractLight, vec3(2.4));
		vec3 refractTotalLight = ft_p_wi_wt * refractLight * wtDotNegN;
		totalLight += refractTotalLight;	
	}

	return totalLight;
}



//���ﶨ�帴���ļ��㷨��
//����һ������
vec2 Complex(float real)
{
	return vec2(real,0);
}

//����ģ��
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


//�����������������һ������ֵ��ʾ�Ľ��ʣ�һ��Ϊ���������������ͷ��ߵļн��Լ���������ڽ��ʺ�������ʵ����������֮�ȣ� ���������ͷ��߼н��·���ⷴ��ı���
float FrenelReflectRatioComplex(float cosTheta_i/*�����ͷ��ߵļн�*/,vec2 etai/*etai.x ��ʾ��������ڽ��ʵ������� ��etai.y��ʾ˥��ϵ��k�� �ø�����ʾn+ik */, vec2 etat/*etat.x ��ʾ������ʵ������� ��etat.y��ʾ˥��ϵ��k�� �ø�����ʾn+ik */){
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

//��������ģ�ͣ���������ݷ������Լ����������ȫ�ԳƷ����ȥ�������Ķ�����ѭfrenel����
vec3 ReflectModelMetalReflect(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){
	vec3 reflectLight = vec3(0);
	vec3 wi = reflect(-wo,n);
	vec3 light = texture(skyTexture,wi).xyz;
	//gama ���룬ת���Կռ�
	light = pow(light, vec3(2.4));
	//���㷴���ı���
	float reflectRatio = FrenelReflectRatioComplex(max(dot(wi,n),0), vec2(1,0),vec2(0.15,3.9));
	reflectLight = reflectRatio * light;
	return reflectLight;
}

//�дֲڶȵ�ʹ��΢�������۵ķ���ģ��
vec3 ReflectModelRoughnessWithMicrofacetTheoryReflect(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){
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
		//��ȡ��ά����㣬Ϊ��theta��phi��
		vec2 samplePoint = HaltonSample2D(sampleIndex); 
		//�������ת��Ϊ�����еķ���
		float theta = samplePoint.x * 2* s_pi;
		float fine = 0.5 * s_pi * samplePoint.y;
		vec3 wi;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalize(wi);

		//��woת���ֲ��ռ�
		vec3 localWo = normalMatrixInverse * normalize(wo); 

		//��ð�������Ϊ΢ƽ��ķ�����
		vec3 halfVec = normalize(wi+localWo);
		float nDotWi = clamp(dot(halfVec,wi),0,1);
		
		//����sparrowģ�͵� brdf�����
		
		float pdf = PDF_Sparrow(halfVec,localWo);//wi��pdf
		float frenel = FrenelReflectRatio(nDotWi,eta) ;//��������
		float unmask = UnMaskAndUnShadow2(localWo,wi);//�Ǽ����ڱκ���Ӱ��
		float curbrdf = pdf* frenel * unmask;

		//����
		wi = normalMatrix * wi;//�任����������
		wi  = normalize(wi);
		vec3 light = texture(skyTexture,wi).xyz;

		//gama ���룬ת���Կռ�
		light = pow(light, vec3(2.4));

		vec3 curLight = curbrdf * light * nDotWi;
		reflectLight+=  curLight * pdf;//���ﲻ��������й�Ļ����ܺͣ��������ÿ����Ӧ�������յĽ����ռ�ݵı��������Բ��ܳ���pdf
		totalPDF +=pdf;


//		//���������BTDF
//		float btdf_p_wo_wi = 1-curbrdf;
//		//����Ӳ����ڲ���������Ĺ��btdf
//		float btdf_p_wi_wo = Ft_P_Wi_Wo(btdf_p_wo_wi,eta);
//		//�����������Ϊ�����ĸ����ܶ�
//		float pdf_refract = PDF_Refract(halfVec,localWo,eta);
//		vec3 wi_refract = refract(-localWo,halfVec,eta);
//		wi_refract = normalize(wi);
//		if(length(wi_refract) == 0)
//		{
//			continue;
//		}
//		//�����������任������ռ�
//		wi_refract = normalMatrix * wi_refract;//�任����������
//		wi_refract  = normalize(wi_refract);
//		light = texture(skyTexture,wi_refract).xyz;
//		light = pow(light, vec3(2.4));
//		refractTotalLight += pdf_refract * btdf_p_wi_wo * light;
//		totalRefractPDF+= pdf_refract;
	}
	reflectLight /=  totalPDF;//��һ��

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
	//gama ����
	//color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);



	//�������ж������Ϊ��ɫ
	//outColor = vec4(1,1,1,1);


}