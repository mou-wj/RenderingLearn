#version 450 core

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的法线
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;

layout(set  =0,binding = 1) uniform sampler3D sigmaTVolumeTexture;
layout(set  =0,binding = 2) uniform Info {
	vec3 cameraPos;
	vec3 lightPos;
	vec3 lightColor;
};
layout(set  =0,binding = 3) uniform sampler3D inScatterVolumeTexture;

const float s_pi = 3.141592653;



//定义相位散射函数

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

//计算透射率,从起始点沿着方向direction进行采样
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

		//将采样点变换到3D纹理空间中的坐标，这里已经假定边界盒为一个正方体，范围分别为-1到1
		if(samplePoint.x <=1 && samplePoint.x >=-1 && 
		   samplePoint.y <=1 && samplePoint.y >=-1 &&
		   samplePoint.z <=1 && samplePoint.z >=-1 ){
				vec3 uvw = (samplePoint + 1) / 2;
				float curSigmaT = texture(sigmaTVolumeTexture,uvw).x;
				validNumSample++;
				//积分
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



	//在该点和光源之间的距离上进行采样
	uint numSample = 30;
	
	float deltaD = 0.1;
	vec3 totalLight = vec3(0);

	//计算当前点的Lo
	float transmittanceLo = Transmittance(inPosition,v);
	float d1 = distance(inPosition,lightPos);
	float attenation1 = 1 / (1 + 0.1 * d1*d1);
	vec3 Lo = lightColor * attenation1 * transmittanceLo;




	for(uint sampleIndex = 0;sampleIndex <numSample;sampleIndex++)
	{
		//float sampleRatio = HaltonSample1D(sampleIndex);
		vec3 samplePoint = inPosition - v * deltaD * sampleIndex;

				//将采样点变换到3D纹理空间中的坐标，这里已经假定边界盒为一个正方体，范围分别为-1到1
		if(samplePoint.x <=1 && samplePoint.x >=-1 && 
		   samplePoint.y <=1 && samplePoint.y >=-1 &&
		   samplePoint.z <=1 && samplePoint.z >=-1 ){

		   	vec3 curL = normalize(lightPos - samplePoint);
			float curD = distance(lightPos,samplePoint);

			//计算相位
			float cosTheta = dot(-curL,v);

			//计算相位因子
			float phaseFactor = PhaseRayleighScattering(cosTheta);

			//计算采样点到相机的透光率
			float transmittanceV = Transmittance(samplePoint,v);
			
			float attenation = 1 / (1 + 0.1 * curD*curD);

			//计算到光源的透光率
			float transmittanceL = Transmittance(samplePoint,curL);


			//计算散射光
			vec3 scatterColor = s_pi * phaseFactor * lightColor * attenation * transmittanceL;


			//计算内散射sigma scatter
			vec3 uvw = (samplePoint + 1) / 2;
			float inSigmaScatter = texture(inScatterVolumeTexture,uvw).x;
			float sigmaScatter = inSigmaScatter;

			//积分
			vec3 curLight = transmittanceV * scatterColor * sigmaScatter * deltaD;
			totalLight += curLight;

		}




		
	
	}

	outColor = vec4(totalLight,1);
									
											
		
}


void DepthFrogExmple(){
	vec3 frogColor = vec3(0.3,0.3,0.3);


	//规定在距离相机1m处开始出现雾

	float df = 0.5;

	//
	float d = distance(inPosition,cameraPos);
	
	//计算透光率
	float f = exp(-df * d);
	vec3 color = (1 - f) * frogColor + f * vec3(1,0,0);//假设物体就为红色

	outColor = vec4(color,1.0);



}

void main(){

	//float a = texture(absorbVolumeTexture,vec3(0,0,0)).x;
	outColor = vec4(1,0,0,1);
	ComputeVolumeScattering();
	//outColor = vec4(inTan,1);
	//DepthFrogExmple();

}