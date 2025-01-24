#version 450 core

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��еķ���
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;


//���弸�����Դ
struct RectLight{
	vec3 pos;
	vec3 rangeMin,rangeMax;
	vec3 normal;
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


layout(set = 0,binding = 1) uniform sampler2D preComputeTexture;


layout(set = 0,binding = 3,std140) uniform Info{
	layout(offset = 16) vec3 cameraPos;//����ռ������λ��
	layout(offset = 32) int exampleType;//ʾ��: 0��ʾGGX�ľ��淴��ʾ��; 1��ʾ�⻬�α���ɢ��ģ��;2��ʾ�ֲڴα���ɢ���ʿ��ģ��;3��ʾ�ֲڴα���ɢ��OrenNayarģ��;4��ʾ���鲼��ģ��;5��ʾ΢���沼��ģ��
};





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
	vec3 l = normalize(pointLightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);
	vec3 h = normalize((l + v) / 2);
	float roughness = 0.1;
	float hDotL = dot(h,l);
	float hDotV = dot(h,v);
	float nDotL = dot(l,inNormal);
	float nDotV = dot(v,inNormal);
	float nDotH = dot(h,inNormal);
	vec3 fdiff = Pss / s_pi;
	vec3 cdiff = s_pi * fdiff * pointLightColor * max(nDotL,0);

	float shininess = 5;
	float fspec = pow(max(nDotH, 0.0), shininess); 
	vec3 cspec = vec3(1,1,1) * (1 - Pss) * fspec;



	outColor = vec4(cdiff + cspec,1.0);



}


void main(){
	outColor = vec4(1,0,0,1);
	vec3 color = texture(preComputeTexture,vec2(0,0)).xyz;


	//GGXSpecularExample();
	BlinnPhongExample();


}