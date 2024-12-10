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
//���ֲ��ʣ��������Li�������Lo����pow(Ni,2) * Lo = pow(No,2) * Li�� �� BTDF ���� pow(Ni,2) * BTDF(p,wi,wo) = pow(No,2) * BTDF(p,wo,wi) 


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


//��������  ���㷴���ı���
float Frenel_Reflect(float cosTheta_i/*�����ͷ��ߵļн�cosֵ*/,float eta/*���������: ����Ĳ��ʽ��ʵ������� / ��������ڵĽ��ʵ������� */){
	float sinTheta_i = sqrt(1 - pow(cosTheta_i,2));
	float cosTheta_t = sqrt(1-pow(sinTheta_i / eta ,2));
	float r_parellel = (eta *  cosTheta_i- cosTheta_t)  / (eta * cosTheta_i + cosTheta_t);
	float r_perpendicular = (cosTheta_i - eta * cosTheta_t)  / (cosTheta_i + eta * cosTheta_t);



	return (pow(r_parellel,2) + pow(r_perpendicular,2) ) / 2;
}

//��������  ���������ı���
float Frenel_Refract(float cosTheta_i/*�����ͷ��ߵļн�cosֵ*/,float eta/*���������: ����Ĳ��ʽ��ʵ������� / ��������ڵĽ��ʵ������� */){
	return 1 - Frenel_Reflect(cosTheta_i,eta);
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

//Sparrow ģ�͵�BRDF������������ھֲ�����ռ���
float BRDF_Sparrow(vec3 wo,vec3 wi,vec3 wm){
	float res = 0;
	float pdf = PDF_Sparrow(wm,wo);
	float cosTheta_i = -wi.y;
	float frenel = Frenel_Reflect(max(cosTheta_i,0),3.5 / 1) ;
	float unmaskAndUnShadow = UnMaskAndUnShadow2(wo,wi);
	res = pdf * frenel * unmaskAndUnShadow ;
	return res;
}

vec3 ExampleSparrowBRDT(vec3 wo,vec3 n)
{
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
	mat3 normalMatrixInverse  = inverse(normalMatrix);

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
		wi = normalize(wi);
		//��woת���ֲ��ռ�
		vec3 localWo = normalMatrixInverse * normalize(wo); 
		//��ð�����
		halfVec = normalize(wi+localWo);

		//��ȡbrdf��
		float curBrdf = BRDF_Sparrow(localWo,wi,halfVec);
		//pdf = PDF_Sparrow(halfVec,wo);
		pdf = PDF_Sparrow(halfVec,localWo);
		float frenel = Frenel_Reflect(max(dot(wi,vec3(0,-1,0)),0),1.5 / 1) ;
		float unmask = UnMaskAndUnShadow2(localWo,wi);
		float curbrdf = pdf* frenel * unmask;
		//reflectLight+=  vec3(curBrdf,0,0);//���ﲻ��������й�Ļ����ܺͣ��������ÿ����Ӧ�������յĽ����ռ�ݵı��������Բ��ܳ���pdf
		//totalPDF +=1;

		//continue;
		wi = normalMatrix * wi;//�任����������
		wi  = normalize(wi);
		light = texture(skyTexture,wi).xyz;

		//gama ���룬ת���Կռ�
		light = pow(light, vec3(2.4));
		nDotWi = clamp(dot(n,wi),0,1);
		vec3 curLight = curBrdf * light * nDotWi;
		reflectLight+=  curLight * pdf;//���ﲻ��������й�Ļ����ܺͣ��������ÿ����Ӧ�������յĽ����ռ�ݵı��������Բ��ܳ���pdf
		totalPDF +=pdf;
	}
	//reflectLight /= numSample;
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
	color = ExampleSparrowBRDT(wo,inNormal);
	//gama ����
	//color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);



	//�������ж������Ϊ��ɫ
	//outColor = vec4(1,1,1,1);


}