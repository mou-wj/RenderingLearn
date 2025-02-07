#version 450 core

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��еķ���
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;


//���弸�����Դ
struct RectLight{
	vec3 p1,p2,p3,p4;
	vec3 normal;
	vec3 color;
};


struct SphereLight{
	vec3 pos;
	vec3 color;
	vec3 radius;
};


struct PipeLight{
	vec3 pos1,pos2;
	vec3 color;
};


vec3 pointLightPos = vec3(0,-3,6);
vec3 pointLightColor = vec3(1,0,0);
const float s_pi = 3.141592653;

const vec3 Pss = vec3(0.3);

layout(set = 0,binding = 2) uniform sampler2D envTexture;

layout(set = 0,binding = 3,std140) uniform Info{
	layout(offset = 16) vec3 cameraPos;//����ռ������λ��
	layout(offset = 32) int exampleType;//ʾ��: 0��ʾGGX�ľ��淴��ʾ��; 1��ʾ�⻬�α���ɢ��ģ��;2��ʾ�ֲڴα���ɢ���ʿ��ģ��;3��ʾ�ֲڴα���ɢ��OrenNayarģ��;4��ʾ���鲼��ģ��;5��ʾ΢���沼��ģ��
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

// ������ά�ռ��е�˫���Բ�ֵ
vec3 bilinearInterpolate(vec3 A, vec3 B, vec3 C, vec3 D, float u, float v) {
    // ˫���Բ�ֵ��ʽ
    return (1.0 - u) * (1.0 - v) * A + u * (1.0 - v) * B + (1.0 - u) * v * D + u * v * C;
}


//������ι�Դ��p���Ĺ�Դ����Ep
void CalculateRectLightVec(in vec3 p,out vec3 Ep,out float LenEp){
	
	vec3 l = vec3(0);
	uint numSample = 20;
	RectLight rectLight;
	rectLight.p1 = vec3(-4,-3,5);
	rectLight.p2 = vec3(-4,-3,7);
	rectLight.p3 = vec3(4,-3,7);
	rectLight.p4 = vec3(4,-3,5);
	

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		vec2 samplePoint = HaltonSample2D(sampleIndex);
		
		vec3 sampleLightPosition = bilinearInterpolate(rectLight.p1,rectLight.p2,rectLight.p3,rectLight.p4,samplePoint.x,samplePoint.y);

		vec3 curLi = normalize(sampleLightPosition - p);

		//����l����
		float d= distance(sampleLightPosition ,p);
		curLi *= 1 / pow(d,2);

		//���Ǿ���˥������Ϊ�ֲ�
		l += curLi;

	}
	LenEp = length(l) / numSample;
	Ep = normalize(l);
}


//���ƾ��ι�Դ��p���Ĺ�Դ����Ep
void DrobotEstimateRectLightVec(in vec3 p,in vec3 n,out vec3 L/*���Ƶ����Դ�ķ���*/,out float LenL){
	
	vec3 l = vec3(0);
	uint numSample = 400;
	RectLight rectLight;
	rectLight.p1 = vec3(-4,-3,5);
	rectLight.p2 = vec3(-4,-3,7);
	rectLight.p3 = vec3(4,-3,7);
	rectLight.p4 = vec3(4,-3,5);
	
	float maxFac = 0;

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		vec2 samplePoint = HaltonSample2D(sampleIndex);
		
		vec3 sampleLightPosition = bilinearInterpolate(rectLight.p1,rectLight.p2,rectLight.p3,rectLight.p4,samplePoint.x,samplePoint.y);

		vec3 curLi = normalize(sampleLightPosition - p);
		float nDotL = max(dot(curLi,n),0);
				//����l����
		float d= distance(sampleLightPosition ,p);
		float fac = nDotL / pow(d,2);

		//�ҵ��������ķ��򲢼�¼
		if(fac > maxFac)
		{
			maxFac = fac;
			L= curLi;
			LenL =  1 / pow(d,2);
		}
	}
}



float Frenel(float nDotL,float eta/*��������ʣ�����Ľ���������/��������ڵĽ��������� */){
	float F0 = pow((eta - 1)/(eta + 1),2); 
	float r = F0 + (1- F0) * pow(1 - max(nDotL,0),5);
	return r;

}



//����ͬ�Ե�GGX PDF
float NDF_GGX_Isotropy(float nDotM,float roughness){
	float res = 0;
	float numerator = max(nDotM,0) * pow(roughness,2);
	float denominator = s_pi * pow( 1 + pow(nDotM,2) * (pow(roughness,2) - 1) ,2);

	res = numerator / denominator;
	return res;
}

//����ͬ�Ե�GGX Lambda����
float LambdaGGX_Isotropy(float nDotS,float roughness){
	float res = 0;
	float a = nDotS / (roughness * sqrt( 1 - pow(nDotS,2) ));
	res = (sqrt(1 + 1 / pow(a,2)) - 1) / 2; 
	return res;


}

//�������Ե�GGX PDF
float NDF_GGX_Anisotropy(float nDotM,float tDotM/*������ռ���m��t�Ĳ��*/,float bDotM/*������ռ���m��b�Ĳ��*/,float roughnessX,float roughnessY){
	float res = 0;
	float numerator = max(nDotM,0);
	float denominator = s_pi * roughnessX * roughnessY * pow( pow(nDotM,2) + pow(tDotM / roughnessX,2) + pow(bDotM / roughnessY,2)  ,2);

	res = numerator / denominator;
	return res;
}



//mask����
float G1(vec3 m,vec3 v,float lambdaV/*v�ӽǵ�lambda�������*/){
	float mDotV = max(dot(m,v),0);
	float res = mDotV / ( 1 + lambdaV);
	return res;
}

//shadow��mask����
float G2Smith(float mDotV,float mDotL,float lambdaV/*v�ӽǵ�lambda�������*/,float lambdaL/*l�ӽǵ�lambda�������*/){
	mDotV = max(mDotV,0);
	mDotL = max(mDotL,0);
	float res = mDotV * mDotL / ( 1 + lambdaV + lambdaL);
	return res;
}


//ʹ��GGX�ֲ�ʵ��һ��ֻ�о��淴��ķ���ģ��
void GGXSpecularExample(){
	//����Ŀǰ��ʾ��ֻ��һ����Դ�����Բ���Ҫ����
	vec3 l = normalize(pointLightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float hDotL = dot(h,l);
	float hDotV = dot(h,v);
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);

	float lambdaV = LambdaGGX_Isotropy(hDotV,roughness);
	float lambdaL = LambdaGGX_Isotropy(hDotL,roughness);


	float ndf_ggx = NDF_GGX_Isotropy(dot(inNormal,h),0.1);
	float frenel =Frenel(nDotL,1.5);
	float g2Smith = G2Smith(hDotV,hDotL,lambdaV,lambdaL);

	float f_spec =  frenel * g2Smith * ndf_ggx / ( 4 * abs(nDotL) * abs(nDotV) );

	vec3 specColor = f_spec * pointLightColor * max(hDotL,0);

	vec3 surface_color = vec3(0.2,0.2,0.2);
	surface_color +=specColor;

	outColor = vec4(surface_color,1.0);

}


//lambdert ����ģ��
void BlinnPhongExample(){
	//����Ŀǰ��ʾ��ֻ��һ����Դ�����Բ���Ҫ����



	vec3 l;
	l = normalize(pointLightPos - inPosition);
	float d = distance(pointLightPos,inPosition);
	float curD = 1 / pow(d,2);
	vec3 Ep;
	float LenEp;
	
	CalculateRectLightVec(inPosition,Ep,LenEp);
	l = Ep;
	curD = LenEp;

	vec3 L;
	float LenL;
	//���ַ��������ڲ���������Ե������յõ��ĸ߹�����Ԥ�ڽ������ƫ��Ӷ����½��������
	//DrobotEstimateRectLightVec(inPosition,inNormal,L,LenL);
	//l = L;
	//curD = LenL;
	
	//LenEp = curD;

	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float hDotL = dot(h,l);
	float hDotV = dot(h,v);
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);
	float nDotH = dot(h,inNormal);
	vec3 fdiff = Pss / s_pi;
	vec3 cdiff = s_pi * fdiff * pointLightColor * max(nDotL,0) * curD;

	float shininess = 30;
	float fspec = pow(max(nDotH, 0.0), shininess); 
	vec3 cspec = vec3(1,1,1) * (1 - Pss) * fspec;



	outColor = vec4(cdiff + cspec ,1.0);



}


void SphereCoordEnvMapExample(){
	

	vec3 n = normalize(inPosition);
	//����u
	float u  = (atan(n.z,n.x) / (s_pi) + 1) / 2;
	float v  = (asin(n.y) / (0.5 * s_pi) + 1) / 2;
	vec3 color = texture(envTexture,vec2(u,v)).xyz;

	color = pow(color, vec3(2.4));
	outColor = vec4(color,1.0);
	

}


void main(){
	outColor = vec4(1,0,0,1);
//	if(exampleType == 0)
//	{
//		BlinnPhongExample();
//	}else if(exampleType == 1){
//		SphereCoordEnvMapExample();
//	}
	SphereCoordEnvMapExample();
	//EnvMapExample();


}