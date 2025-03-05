#version 450 core

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��еķ���
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

layout(set = 0,binding = 1,std140) uniform Info{
	layout(offset = 0) vec3 cameraPos;//����ռ������λ��
	layout(offset = 16) vec3 lightDir;
	layout(offset = 32) mat4 lightTransform;
};



layout(set = 0,binding = 2) uniform sampler2D baseColorTexture;
layout(set = 0,binding = 3) uniform sampler2D depthTexture;

//˫���Բ�ֵ
vec4 BilinearInterpolate(vec4 f00, vec4 f10, vec4 f01, vec4 f11, float u, float v) {
    return mix(
        mix(f00, f10, u), // �� x �����ֵ
        mix(f01, f11, u), // �� x �����ֵ
        v                 // �� y �����ֵ
    );
}

vec3 LightShade(){
	vec3 res = vec3(0);
	//����lambert��ɫ

	vec3 v = normalize(cameraPos - inPosition);
	vec3 l = lightDir;
	vec3 h = normalize(v + l);

	//����������
	const float Pss = 0.4;//����������ϵ��
	const float shiness = 10;//���ø߹�ϵ��
	const vec3 lightColor = vec3(1,1,1);

	vec3 baseColor = texture(baseColorTexture,inTexCoord.xy * vec2(1,-1)).xyz;
	baseColor = pow(baseColor,vec3(2.2));

	vec3 diff = baseColor * Pss * lightColor;

	//����߹���
	float nDotH = max(0,dot(h,inNormal));
	vec3 spec = (1 - Pss) * pow(nDotH,10) * lightColor;

	res = diff + spec;
	return res;

}

float CalculateVisibilityPCSS(vec2 uv,float minDepth,float realDepth){
	

	//�����Դ���ڵ���֮��ľ��� 
	float dloccluder = minDepth;
	
	//�����Դ�ͽ�������֮��ľ��� 
	float dlobject = realDepth;


	float totalVisibility =0;

	uint filterWH = 4;//����6x6������


	//���ݾ��붯̬���������С
	//float d0 = curLightDirMinFragDepth;
	//float dr =  realFragDepthInLightView;
	float d0 = dloccluder;
	float dr =  dlobject;
	float ratio = (dr - d0) / d0;
	float w = filterWH * ratio;
	filterWH = clamp(uint(w),1,2*filterWH);

	vec2 depthTextureSize = textureSize(depthTexture,0);
	vec2 deltaUV = vec2(1) / vec2(depthTextureSize.xy);

	for(uint i = 0;i < filterWH;i++)
	{
		for(uint j = 0;j < filterWH;j++)
		{
			vec4 visibility;
			vec2 sampleUV = uv - deltaUV * vec2(i,j);

			//�����ĸ���
			vec4 fourSampleX = textureGather(depthTexture,sampleUV);

			for(uint pid = 0;pid < 4;pid++)
			{
				
				if(fourSampleX[pid] + 0.00001 < realDepth)//����һ��ƫ��
				{
					//outColor = vec4(0,0,0,1);
					visibility[pid] = 0;
				}else {
					visibility[pid] = 1;
				}
			
			
			}
			vec2 localUV = fract(sampleUV * depthTextureSize.xy);

				//˫���Բ�ֵ
			vec4 color = BilinearInterpolate(vec4(visibility.x,0,0,1),
			vec4(visibility.y,0,0,1),
			vec4(visibility.z,0,0,1),
			vec4(visibility.w,0,0,1),
			localUV.x,
			localUV.y
			);
			totalVisibility +=color.x;
		}
	
	
	}

	totalVisibility/= filterWH * filterWH;
	return totalVisibility;
}

void CalculateShade(){
	//�����ڹ�Դ�ռ��е�����
	vec4 coordInLightSpace = lightTransform * vec4(inPosition,1.0);

	//�����ڹ�Դ�ռ��е����
	vec2 uv = (coordInLightSpace.xy +1)/2;
	
	float minDepth = texture(depthTexture,uv).x;

	vec3 shadeColor = LightShade();


	float visibility = 1;
	if(minDepth + 0.00001 < coordInLightSpace.z)
	{
		//����Ӱ��,�����Ƿ���⣬����û������Ӱ
		visibility = CalculateVisibilityPCSS(uv,minDepth,coordInLightSpace.z);
		//visibility = 0.2;
	}
	vec3 finalShadeColor = shadeColor * visibility ;

	const vec3 frogColor = vec3(1,1,1);
	float alpha = 0;
	float disVP = distance(cameraPos,inPosition);
	if(disVP > 3)
	{
		alpha = clamp((disVP - 3) / 50,0,0.9);
	}



	outColor = vec4(finalShadeColor,alpha);
}

void main(){
	vec3 sampleColor = texture(baseColorTexture,inTexCoord.xy * vec2(1,-1)).xyz;
	sampleColor = pow(sampleColor,vec3(2.2));
	outColor = vec4(1,0,0,1);
	outColor = vec4(sampleColor,1.0);
	CalculateShade();
}