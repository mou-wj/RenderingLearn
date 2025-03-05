#version 450 core

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的法线
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

layout(set = 0,binding = 1,std140) uniform Info{
	layout(offset = 0) vec3 cameraPos;//世界空间中相机位置
	layout(offset = 16) vec3 lightDir;
	layout(offset = 32) mat4 lightTransform;
};



layout(set = 0,binding = 2) uniform sampler2D baseColorTexture;
layout(set = 0,binding = 3) uniform sampler2D depthTexture;

//双线性插值
vec4 BilinearInterpolate(vec4 f00, vec4 f10, vec4 f01, vec4 f11, float u, float v) {
    return mix(
        mix(f00, f10, u), // 在 x 方向插值
        mix(f01, f11, u), // 在 x 方向插值
        v                 // 在 y 方向插值
    );
}

vec3 LightShade(){
	vec3 res = vec3(0);
	//采用lambert着色

	vec3 v = normalize(cameraPos - inPosition);
	vec3 l = lightDir;
	vec3 h = normalize(v + l);

	//计算漫反射
	const float Pss = 0.4;//设置漫反射系数
	const float shiness = 10;//设置高光系数
	const vec3 lightColor = vec3(1,1,1);

	vec3 baseColor = texture(baseColorTexture,inTexCoord.xy * vec2(1,-1)).xyz;
	baseColor = pow(baseColor,vec3(2.2));

	vec3 diff = baseColor * Pss * lightColor;

	//计算高光项
	float nDotH = max(0,dot(h,inNormal));
	vec3 spec = (1 - Pss) * pow(nDotH,10) * lightColor;

	res = diff + spec;
	return res;

}

float CalculateVisibilityPCSS(vec2 uv,float minDepth,float realDepth){
	

	//计算光源和遮挡物之间的距离 
	float dloccluder = minDepth;
	
	//计算光源和接受物体之间的距离 
	float dlobject = realDepth;


	float totalVisibility =0;

	uint filterWH = 4;//定义6x6的网格


	//根据距离动态调整网格大小
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

			//采样四个点
			vec4 fourSampleX = textureGather(depthTexture,sampleUV);

			for(uint pid = 0;pid < 4;pid++)
			{
				
				if(fourSampleX[pid] + 0.00001 < realDepth)//加上一定偏移
				{
					//outColor = vec4(0,0,0,1);
					visibility[pid] = 0;
				}else {
					visibility[pid] = 1;
				}
			
			
			}
			vec2 localUV = fract(sampleUV * depthTextureSize.xy);

				//双线性插值
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
	//计算在光源空间中的坐标
	vec4 coordInLightSpace = lightTransform * vec4(inPosition,1.0);

	//采样在光源空间中的深度
	vec2 uv = (coordInLightSpace.xy +1)/2;
	
	float minDepth = texture(depthTexture,uv).x;

	vec3 shadeColor = LightShade();


	float visibility = 1;
	if(minDepth + 0.00001 < coordInLightSpace.z)
	{
		//在阴影中,由于是方向光，所以没有软阴影
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