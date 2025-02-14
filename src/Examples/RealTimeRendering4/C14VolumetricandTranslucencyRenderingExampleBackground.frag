#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

layout(set  =0,binding = 1) uniform Info {
	vec3 cameraPos;
	vec3 lightPos;
	vec3 lightColor;
};
void ComputePointLightVolume(){

	vec3 v = normalize(cameraPos-inPosition);
	

	uint numSample = 30;
	float d = distance(inPosition,cameraPos);
	float deltaD = d / numSample;

	float sigmaT = 0.1;
	vec3 totalLight = vec3(0);

	for(uint sampleIndex =0;sampleIndex<numSample;sampleIndex++)
	{
		vec3 samplePos = inPosition + sampleIndex * deltaD * v;
		vec3 curL = normalize(lightPos - samplePos);
		float curDl = distance(lightPos, samplePos);
		float curDv = distance(cameraPos,samplePos);
	
		//相位因子为常量
		float phaseFactor = 0.25 * s_pi;

		//计算采样点到相机的透光率
		float transmittanceV = exp(-sigmaT * curDv);
		
		float attenation = 1 / (1 + 3 * curDl*curDl);

		//计算到光源的透光率
		float transmittanceL = exp(-sigmaT * curDl);


		//计算散射光
		vec3 scatterColor = s_pi * phaseFactor * lightColor * attenation * transmittanceL;


		//计算内散射sigma scatter
		float sigmaScatter = 0.4;

		//积分
		vec3 curLight = transmittanceV * scatterColor * sigmaScatter * deltaD;
		totalLight += curLight;

	}

	outColor = vec4(totalLight,1.0);

}

void main(){

	//float a = texture(absorbVolumeTexture,vec3(0,0,0)).x;
	outColor = vec4(1,0,0,1);
	ComputePointLightVolume();

}