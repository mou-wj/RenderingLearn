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


float s_LamberRefectionFactor = 0.4;//lambertģ�ͷ�����
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

vec3 LambertBSDF(vec3 wo,vec3 n){
	
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

		reflectLight+= BSDFLambert(wo,wi) * light * dot(n,wi) / PDFLambert(wi,wo)/numSample;


	}

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

vec3 SimpleSpecularBRDF(vec3 wo, vec3 n){
	uint numSample = 16;
	vec2 samplePoint;
	vec3 wi;
	vec3 light;
	vec3 reflectLight = vec3(0);
	vec3 halfVec;

	float dw = 2 * s_pi / numSample;
	//��������ռ�ķ������������ϰ�������ϵ
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
		if(pdf != 0)
		{
			nDotWi = clamp(dot(n,wi),0,1);
			vec3 curLight = BRDFSimpleSpecular(wo,wi) * light * nDotWi;
			reflectLight+=  curLight / pdf;
		
		}
	}
	reflectLight /= numSample;

	return reflectLight;

}






void main(){

	vec3 wo = normalize(viewPosition - inWorldPosition);
	vec3 color;
	//color = LambertBSDF(wo,inNormal);
	color = SimpleSpecularBRDF(wo,inNormal);
	//color = texture(skyTexture,inNormal).xyz;
	//gama ����
	//color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);



	//�������ж������Ϊ��ɫ
	//outColor = vec4(1,1,1,1);


}