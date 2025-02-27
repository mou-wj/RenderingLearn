#version 450 core

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��еķ���
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;

layout(set  =0,binding = 1) uniform sampler3D sigmaTVolumeTexture;
layout(set  =0,binding = 2) uniform Info {
	vec3 cameraPos;
	vec3 lightPos;
	vec3 lightColor;
};
layout(set  =0,binding = 3) uniform sampler3D inScatterVolumeTexture;

const float s_pi = 3.141592653;



//������λɢ�亯��

float PhaseRayleighScattering(float cosTheta){
	float res  =0;
	float cosTheta2 = pow(cosTheta,2);
	res = 3 * (1 + cosTheta2) / (16 *s_pi);
	return res;
}


float PhaseMieScatteringSchlick(float cosTheta,float g){
	float res  =0;
	float cosTheta2 = pow(cosTheta,2);
	float k = 1.55 * g - 0.55 * pow(g,3);
	res = (1 - pow(k,2)) / (4 * s_pi * pow(1 + k * cosTheta ,2));
	return res;
}

//����͸����,����ʼ�����ŷ���direction���в���
float Transmittance(vec3 startPos,vec3 direction){
	float res = 1;

	uint numSample = 30;
	float d = distance(cameraPos,inPosition);
	float sigmaT = 0;
	float deltaD = d / numSample;
	uint validNumSample = 0;
	for(uint sampleIndex = 0;sampleIndex <numSample;sampleIndex++)
	{
		
		vec3 samplePoint = startPos + direction * deltaD *sampleIndex;

		//��������任��3D����ռ��е����꣬�����Ѿ��ٶ��߽��Ϊһ�������壬��Χ�ֱ�Ϊ-1��1
		if(samplePoint.x <=1 && samplePoint.x >=-1 && 
		   samplePoint.y <=1 && samplePoint.y >=-1 &&
		   samplePoint.z <=1 && samplePoint.z >=-1 ){
				vec3 uvw = (samplePoint + 1) / 2;
				float curSigmaT = texture(sigmaTVolumeTexture,uvw).x;
				validNumSample++;
				//����
				sigmaT +=curSigmaT;
		}
	}
	if(validNumSample == 0)
	{
		res = 1;
		
	}else {
		deltaD = d / validNumSample;
		res = exp(-sigmaT * deltaD);
	}

	return res;
}





void ComputeVolumeScattering(){
	vec3 l = normalize(lightPos - inPosition);
	vec3 v = normalize(cameraPos - inPosition);
	float d = distance(lightPos,inPosition);



	//�ڸõ�͹�Դ֮��ľ����Ͻ��в���
	uint numSample = 30;
	
	float deltaD = 0.1;
	vec3 totalLight = vec3(0);

	//���㵱ǰ���Lo
	float transmittanceLo = Transmittance(inPosition,v);
	float d1 = distance(inPosition,lightPos);
	float attenation1 = 1 / (1 + 0.1 * d1*d1);
	vec3 Lo = lightColor * attenation1 * transmittanceLo;




	for(uint sampleIndex = 0;sampleIndex <numSample;sampleIndex++)
	{
		//float sampleRatio = HaltonSample1D(sampleIndex);
		vec3 samplePoint = inPosition - v * deltaD * sampleIndex;

				//��������任��3D����ռ��е����꣬�����Ѿ��ٶ��߽��Ϊһ�������壬��Χ�ֱ�Ϊ-1��1
		if(samplePoint.x <=1 && samplePoint.x >=-1 && 
		   samplePoint.y <=1 && samplePoint.y >=-1 &&
		   samplePoint.z <=1 && samplePoint.z >=-1 ){

		   	vec3 curL = normalize(lightPos - samplePoint);
			float curD = distance(lightPos,samplePoint);

			//������λ
			float cosTheta = dot(-curL,v);

			//������λ����
			float phaseFactor = PhaseRayleighScattering(cosTheta);

			//��������㵽�����͸����
			float transmittanceV = Transmittance(samplePoint,v);
			
			float attenation = 1 / (1 + 0.1 * curD*curD);

			//���㵽��Դ��͸����
			float transmittanceL = Transmittance(samplePoint,curL);


			//����ɢ���
			vec3 scatterColor = s_pi * phaseFactor * lightColor * attenation * transmittanceL;


			//������ɢ��sigma scatter
			vec3 uvw = (samplePoint + 1) / 2;
			float inSigmaScatter = texture(inScatterVolumeTexture,uvw).x;
			float sigmaScatter = inSigmaScatter;

			//����
			vec3 curLight = transmittanceV * scatterColor * sigmaScatter * deltaD;
			totalLight += curLight;

		}




		
	
	}

	outColor = vec4(totalLight,1);
									
											
		
}


void DepthFrogExmple(){
	vec3 frogColor = vec3(0.3,0.3,0.3);


	//�涨�ھ������1m����ʼ������

	float df = 0.5;

	//
	float d = distance(inPosition,cameraPos);
	
	//����͸����
	float f = exp(-df * d);
	vec3 color = (1 - f) * frogColor + f * vec3(1,0,0);//���������Ϊ��ɫ

	outColor = vec4(color,1.0);



}

void main(){

	//float a = texture(absorbVolumeTexture,vec3(0,0,0)).x;
	outColor = vec4(1,0,0,1);
	ComputeVolumeScattering();
	//outColor = vec4(inTan,1);
	//DepthFrogExmple();

}