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



//frenel ���� Ni * sin(theta_i) = No * sin(theta_o)  ����Ni No��ʾ���Ե������ʣ�theta_i theta_o��ʾ���Ժͷ���֮��ļн�
//���ֲ��ʣ��������Li�������Lo����pow(Ni,2) * Lo = pow(No,2) * Li�� �� BTDF ���� pow(No,2) * BTDF(p,wi,wo) = pow(Ni,2) * BTDF(p,wo,wi) 



//΢������������ʾ�ֲڶ�

float roughnessX = 0.03,roughnessY = 0.03;//GGX��theta��fine����ά�ȵĴֲڶȿ���ϵ��
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
	float pdfWm = PDF_GGX(w);
	float unmask = UnMask(wo);

	return unmask * pdfWm * max(0,dot(w,wo)) / cos(fine);

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

//����΢����ֲ������ش�wo���������������wi�ķֲ�,�Ƶ���Դ����Microfacet Models for Refraction through Rough Surfaces��ƪ����
float PDF_Refract(vec3 w,vec3 wo,float eta/*���������: ��������ڵĲ��ʽ��ʵ�������ni  / ��������ڵĽ��ʵ�������no*/){
	//����

	//��ȡ��wo����wm��pdf
	float pdf = 0;
	//ʹ�ú�wo��صķ��߷ֲ�
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


	//���㷴���Ĺ���
	totalLight+= reflectRatio* reflectLight;

	
	//��������ⷽ��
	vec3 wt = refract(-wo,n,1/1.5);
	if(length(wt)!=0)//��������
	{
		vec3 refractSampleLight = texture(skyTexture,wt).xyz;
		//gama ���룬ת���Կռ�
		refractSampleLight = pow(refractSampleLight, vec3(2.4));
		totalLight += refractSampleLight * refractRatio;
		
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

//����һ�����Ͼ��ȷֲ��ĸ��Χ��-1��1֮�����������ϱ任�õ�һ������΢ƽ�淴���ķ�������������֮���theta��fine��ֵ�ķֲ����������ֵ��һ��Ϊtheta�Ĳ�ֵ����Χ-pi��pi���ڶ���Ϊfine�Ĳ�ֵ����Χ-pi/2 ��pi/2
vec2 FittingInverseCDFSparrowReflect(vec2 daltaXY/*�����ֱ�Ϊ-1��1֮��Ķ�ά����*/){
	vec2 res;
	//��һ���ʾ��Ŀ��������theta�ǵĲ�ֵ����Χ-pi��pi
	res.x = asin(pow(daltaXY.x,3)) * 2;
	res.y = asin(pow(daltaXY.y,3));


	return res;
}


//�дֲڶȵ�ʹ��΢�������۵ķ���ģ��
vec3 ReflectModelRoughnessWithMicrofacetTheoryReflect(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){
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

	//�����������任���������ı�������ϵ��
	vec3 wr = reflect(-wo,n);
	wr = normalize(normalMatrixInverse * wr);
	//����wr�ڱ�������ϵ�е�thetaֵ��fineֵ
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
		//��ȡ��ά����㣬Ϊ��theta��phi��
		vec2 samplePoint = HaltonSample2D(sampleIndex); 


		//��Ҫ�Բ����ֲ����������������theta��fineֵ,ʹ��inverse CDF�Ƚ��鷳�����ü��㣬�����ʹ�ò��������ͷ�������������ֵ�����ȡ��������
		samplePoint -=0.5;
		samplePoint *=2;//�䵽-1��1֮��
		vec2 deltaThetaFine = FittingInverseCDFSparrowReflect(samplePoint);
			
		//���ݷ���������theta��fineֵ�Լ��任���������ͷ�������theta��fine�Ĳ�ֵ�������������theta��fineֵ
		float theta = deltaThetaFine.x + theta_r;
		float fine = deltaThetaFine.y + fine_r;

		vec3 wi;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalize(wi);

		//��woת���ֲ��ռ�
		vec3 localWo = normalize( normalMatrixInverse * normalize(wo)); 
		
		//��ð�������Ϊ΢ƽ��ķ�����
		vec3 halfVec = normalize(wi+localWo);
		float nDotWi = clamp(dot(halfVec,wi),0,1);
		if(nDotWi ==0)//��ǰΪ��Ч����
		{
			continue;
		}


		
		//����sparrowģ�͵� brdf�����
		
		float pdf = PDF_Sparrow(halfVec,localWo);//wi��pdf
		if(pdf == 0)
		{
			continue;
		}


		float frenel = FrenelReflectRatio(nDotWi,eta) ;//��������
		float unmask = UnMaskAndUnShadow2(localWo,wi);//�Ǽ����ڱκ���Ӱ��
		float curbrdf = pdf* frenel * unmask / nDotWi;

		//����
		wi = normalMatrix * wi;//�任����������
		wi  = normalize(wi);
		vec3 light = texture(skyTexture,wi).xyz;


		//gama ���룬ת���Կռ�
		light = pow(light, vec3(2.4));
		float curWeight = curbrdf * nDotWi;//Ȩ��,����sparrowģ�͵�brdf�к���pdf����Բ����ٳ���pdf��ͨ����Ҫ�Բ����ĵķ�ʽ�������⣬�ٳ���pdf���ظ�������pdf��Ҳ��ʧȥpdf������
		vec3 curLight = curWeight * light;
		reflectLight+=  curLight;
		totalWeight += curWeight;
	}
	reflectLight /=  totalWeight;//��һ��
	return reflectLight;


}

//�дֲڶȵ�ʹ��΢�������۵�����ģ��
vec3 ReflectModelRoughnessWithMrcrofacetTheoryRefract(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){
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


	//��������������theta��fine
	vec3 wt = refract(-wo,n,no/ni);
	wt = normalize(normalMatrixInverse * wt);

	//��������ת���ֲ��ռ�
	wo = normalize(normalMatrixInverse * wo);

	

	//����wt�ڱ�������ϵ�е�thetaֵ��fineֵ
	float theta_t = atan(wt.z,wt.x);
	if(theta_t <0)
	{
		theta_t+=2* s_pi;
	}
	float fine_t = acos(-wt.y) ; 

	uint numSample = 20;
	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//��ȡ��ά����㣬Ϊ��theta��phi��
		vec2 samplePoint = HaltonSample2D(sampleIndex); 


		//��Ҫ�Բ����ֲ����������������theta��fineֵ,ʹ��inverse CDF�Ƚ��鷳�����ü��㣬�����ʹ�ò��������ͷ�������������ֵ�����ȡ��������
		samplePoint -=0.5;
		samplePoint *=2;//�䵽-1��1֮��
		vec2 deltaThetaFine = FittingInverseCDFSparrowReflect(samplePoint);
			
		//��������������theta��fineֵ�Լ��任���������ͷ�������theta��fine�Ĳ�ֵ�������������theta��fineֵ
		float theta = deltaThetaFine.x + theta_t;
		float fine = deltaThetaFine.y + fine_t;

		vec3 wi;
		wi.x = cos(theta)* sin(fine);
		wi.z = sin(theta)* sin(fine);
		wi.y = -cos(fine);
		wi = normalize(wi);

		//����wm
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
				//�����������
				float frenel = FrenelReflectRatio(woDotWm,no/ni);
				float refractRatio = 1-frenel;
				float ft_p_wo_wi = wtPdf * unmask * refractRatio / wiDotWm;
				//����
				wi = normalMatrix * wi;//�任����������
				wi  = normalize(wi);
				vec3 light = texture(skyTexture,wi).xyz;
				//gama ���룬ת���Կռ�
				light = pow(light, vec3(2.4));
				float curWeight = ft_p_wo_wi * wiDotWm;
				//vec3 cl = curWeight * light ;
				refractLight += light * curWeight;
				totalWeight += curWeight;
				
			}
	

		}

	}



	
	refractLight /=  totalWeight;//��һ��

	return refractLight;


}

//�дֲڶȵ�ʹ��΢�������۵ķ��������ģ��
vec3 ReflectModelRoughnessWithMrcrofacetTheoryReflectAndRefract(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){
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

	//�������뷴�����������

	//�����������任���������ı�������ϵ��
	vec3 wr = reflect(-wo,n);
	wr = normalize(normalMatrixInverse * wr);
	//����wr�ڱ�������ϵ�е�thetaֵ��fineֵ
	float theta_r = atan(wr.z,wr.x);
	if(theta_r <0)
	{
		theta_r+=2* s_pi;
	}
	float fine_r = acos(-wr.y) ; 


	//��������������theta��fine
	vec3 wt = refract(-wo,n,no/ni);
	wt = normalize(normalMatrixInverse * wt);

	//����wt�ڱ�������ϵ�е�thetaֵ��fineֵ
	float theta_t = atan(wt.z,wt.x);
	if(theta_t <0)
	{
		theta_t+=2* s_pi;
	}
	float fine_t = acos(-wt.y) ; 

	//��������ת���ֲ��ռ�
	wo = normalize(normalMatrixInverse * wo);
	float refractTotalWeight = 0,reflectTotalWeight = 0;

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		//��ȡ��ά����㣬Ϊ��theta��phi��
		vec2 samplePoint = HaltonSample2D(sampleIndex); 


		//��Ҫ�Բ����ֲ����������������theta��fineֵ,ʹ��inverse CDF�Ƚ��鷳�����ü��㣬�����ʹ�ò��������ͷ�������������ֵ�����ȡ��������
		samplePoint -=0.5;
		samplePoint *=2;//�䵽-1��1֮��
		vec2 deltaThetaFine = FittingInverseCDFSparrowReflect(samplePoint);
			
		//���㷴��
		if(true){
			float sampleTheta = theta_r+deltaThetaFine.x;
			float sampleFine = fine_r+deltaThetaFine.y;
			vec3 wi;
			wi.x = cos(sampleTheta)* sin(sampleFine);
			wi.z = sin(sampleTheta)* sin(sampleFine);
			wi.y = -cos(sampleFine);
			wi = normalize(wi);

		
			//��ð�������Ϊ΢ƽ��ķ�����
			vec3 halfVec = normalize(wi+wo);
			float nDotWir = clamp(dot(halfVec,wi),0,1);
		
			//����sparrowģ�͵� brdf�����
		
			float pdf = PDF_Sparrow(halfVec,wo);//wi��pdf
			if(pdf != 0 && nDotWir!=0)
			{
				float frenel = FrenelReflectRatio(nDotWir,no / ni) ;//��������
				float unmask = UnMaskAndUnShadow2(wo,wi);//�Ǽ����ڱκ���Ӱ��
				float curbrdf = pdf* frenel * unmask / nDotWir;

				//����
				wi = normalMatrix * wi;//�任����������
				wi  = normalize(wi);
				vec3 light = texture(skyTexture,wi).xyz;


				//gama ���룬ת���Կռ�
				light = pow(light, vec3(2.4));
				float curWeight = curbrdf * nDotWir;//Ȩ��,����sparrowģ�͵�brdf�к���pdf����Բ����ٳ���pdf��ͨ����Ҫ�Բ����ĵķ�ʽ�������⣬�ٳ���pdf���ظ�������pdf��Ҳ��ʧȥpdf������
				vec3 curLight = light * curWeight;
				totalWeight += curWeight;
				totalLight+= curLight;
				reflectTotalWeight+=curWeight;
			}		
		
		}

		//��������
		if(true){

			float sampleTheta = theta_t+deltaThetaFine.x;
			float sampleFine = fine_t+deltaThetaFine.y;

			vec3 wi;
			wi.x = cos(sampleTheta)* sin(sampleFine);
			wi.z = sin(sampleTheta)* sin(sampleFine);
			wi.y = -cos(sampleFine);
			wi = normalize(wi);

			//����wm
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
					//�����������
					float frenel = FrenelReflectRatio(woDotWm,no/ni);
					float refractRatio = 1-frenel;
					float ft_p_wo_wi = wtPdf * unmask * refractRatio / wiDotWm;
					//����
					wi = normalMatrix * wi;//�任����������
					wi  = normalize(wi);
					vec3 light = texture(skyTexture,wi).xyz;
					//gama ���룬ת���Կռ�
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


	totalLight/=totalWeight;//��һ��

	//vec3 res = (reflectLight + refractLight)/(reflectTotalWeight + refractTotalWeight);
	return totalLight; 


}

//���ʵ�brdfͨ�������õ��ķ���ģ��  
vec3 ReflectModelMesuredBRDF(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){

	vec3 res;
	//to do  Ŀǰ���Ҫ���mesured bsdf��ʾ������Ҫ�˽�.bsdf�����ļ���ʽ�Լ��������ݱ�����˼����Ҫһ��ʱ�䣬������һ������ʱ����
	return res;

}



//ͷ���ķ���ģ��  

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

//Mp(wo,wi)  ,��������ϵ��
float HairLongitudinalScatteringFunction(vec3 wo,vec3 wi,float v/*�ֲڶ�*/){
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



//���������������ƣ��Ҳ���ͷ��˿��Բ����ģ��ģ�ͣ����ҵ��Ļ���������������ʾ��Ȼ���������Ⱦ������ͷ������ʾ������������������ֱ����һ��ϸԲ����ʾһ��ͷ��Ȼ������֤
//���Բ���������ߵķ�������Ϊ(0,-1,0),�ʷ�ƽ���Ϊxzƽ�� Բ��ֱ��Ϊ0.1
vec3 ReflectModelForHair(vec3 wo/*����ռ��еĳ�������*/,vec3 n/*����ռ��еķ�����*/){
	//ͷ��ģ�͵�6������
	float h=0;//��������wo��Բ���Ĵ�ֱֱ���ϵ�ͶӰ��Ͱ뾶�ı�������ΧΪ-1��1
	float eta = 0;//ͷ����������
	float sigma_a = 0;//ͷ��������ϵ��
	float beta_m = 0;//��Բ������н���صĴֲڶȣ���Χ0��1
	float beta_n = 0;//��Բ�������߼н���صĴֲڶȣ���Χ0��1
	float alpha = 0;//ͷ��������ʺ�Բ����Եĸ�ߵļнǣ�һ��Ϊ2��

	float v = 0.02;//�ֲڶ�,����ֱ�Ӷ��壬ʵ���ϸ�ֵӦ��ͨ��beta_m����õ�
	vec3 alongD = vec3(0,0,1);//����ռ��е�ͷ����������

	//�鿴�Ƿ�n��alongD��ֱ
	float nDotAlongD = dot(alongD,n);
	vec3 res;
	n = normalize(n);
	wo = normalize(wo);


	//����alongD��n������������ϵ������alongD��Ϊz��n��Ϊ-y
	vec3 y = -normalize(n);
	vec3 z = normalize(alongD);
	vec3 x = normalize(cross(y,z));
	mat3 localMatrix  = mat3(x,y,z);
	mat3 inverseMatrix = inverse(localMatrix);
	//��ȡ���������
	vec3 wri = reflect(-wo,n);
	//��wri��woת������������ϵ
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
	
	//gama ����
	//color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);



	//�������ж������Ϊ��ɫ
	//outColor = vec4(1,1,1,1);


}