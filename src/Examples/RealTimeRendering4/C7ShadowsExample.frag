#version 450 core

//�������⣺
//SAT��������ȫ0������
//������ͼ��ȷ����֤
//�Ӳ���ͼ
//�����Դ

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��е�λ��
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;


layout(set = 0,binding = 1) uniform samplerCube pointLightDepthTexture;
layout(set = 0,binding = 2,std140) uniform SceenInfo{
	vec3 pointLightPos;
	layout(offset = 16) vec3 cameraPos;
	layout(offset = 32) mat4 pointLightTransformM[6];//�ֱ�Ϊ+x,-x,+y,-y,+z,-z���������MVP�任����
	uint exampleType;//��ʾҪ��ʾ����Ӱ�㷨ʾ����0:��ʾֱ��ͨ����Ӱ��ͼ; 1:PCF �ٷֱ��˲�; 2: PCSS -PCF�Ľ��汾�����ݾ�����Ϣ�����˲��˴�С; 3:CVM ������Ӱ��ͼ 

};
layout(set = 0,binding = 3) uniform sampler2DArray pointLightDepthTextureArray;


//ƽ��ͶӰ��Ӱ����Ҫԭ��Ϊ���ڵ���ͨ��ͶӰ����ķ�ʽ�������ƽ���ϵ�ͶӰ�㣬Ȼ��ͶӰ���ڱ任��ƽ����������Ⱦ�д�������ͨ��������ɫ������ͶӰ����ɵ������εȷ�ʽ������ʹ�������Լ�������ʽ�����ﲻչ������
void PlaneCastShadowExample(){}

//��Ӱ����Ӱ����Ҫԭ��Ϊÿ�������θ��ݹ�Դ���������һ����Ӱ�壬������۾��з�����Ĺ����մ��������Ӱ�嵽��ָ��λ�ã���˵�����λ�ò�����Ӱ�У���֮������Ӱ��
void ShadowVolumeExample(){}

void GetMatrixIndex(vec3 sampleV,out uint layer,out vec2 uv){
	float absx = abs(sampleV.x);
	float absy = abs(sampleV.y);
	float absz = abs(sampleV.z);
	float u = 0,v = 0;
	if(absx >=absy && absx >= absz)
	{

		if(sampleV.x >=0){
			//�ֶ�����uv
			u = (-sampleV.z / absx + 1) /2 ;
			v  = (sampleV.y / absx + 1) / 2;
			layer = 0;
			
		}else {
			u = (sampleV.z  / absx  + 1) /2 ;
			v  = (sampleV.y  / absx + 1) / 2;
			layer = 1;
			
		}
	
	}

	if(absy >=absx && absy >= absz)
	{
		if(sampleV.y >=0){
			u = (sampleV.x / absy  + 1) /2 ;
			v  = (-sampleV.z / absy + 1) / 2;
			layer = 2;
			
		}else {
			u = (sampleV.x  / absy + 1) /2 ;
			v  = (sampleV.z / absy + 1) / 2;
			layer = 3;
			
		}
	
	}

	if(absz >=absx && absz >= absy)
	{
		if(sampleV.z >=0){
			u = (sampleV.x  / absz + 1) /2 ;
			v  = (sampleV.y / absz+ 1) / 2;
			layer = 4;
			
		}else {
			u = (-sampleV.x / absz + 1) /2 ;
			v  = (sampleV.y / absz + 1) / 2;
			layer = 5;
			
		}
	
	}
	//v = -v;//��תv
	uv = vec2(u,v);

}

//��Ӱ��ͼ��Ӱ����Ҫ���ù��������Ӱ��ͼ���ж�ĳһ��λ���Ƿ�����Ӱ�У�Ӳ��Ӱ
void ShadowMapExample(){

	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);
	float curLightDirMinFragDepth = texture(pointLightDepthTexture,lightViewDir).x;//ֱ��ʹ��cube�������

	uint layer = 0;
	vec2 uv;
	
	//����ǰƬ��ת����Դ�ı任�����м������Ӧ���
	GetMatrixIndex(lightViewDir,layer,uv);
	vec2 curuv = gl_FragCoord.xy / vec2(512,512);

	vec3 depthTextureSize = textureSize(pointLightDepthTextureArray,0);
	//float s = texelFetch(pointLightDepthTexture,ivec3(ivec2(uv),layer));
	float curLightDirMinFragDepthUV = texelFetch(pointLightDepthTextureArray,ivec3(ivec2(uv * depthTextureSize.xy),layer),0).x;

	vec4 realFragPosInLightView = pointLightTransformM[layer] * vec4(inPosition,1.0);
	realFragPosInLightView /=realFragPosInLightView.w;
	float realFragDepthInLightView = realFragPosInLightView.z;

	if(curLightDirMinFragDepthUV + 0.0001 < realFragDepthInLightView)//����һ��ƫ��
	{
		outColor = vec4(0,0,0,1);
	
	}




}

//͸��������Ӱ��ͼ
//ԭ��: ���ݳ����е���������֮��ľ��룬��������Ӱ��ͼ��͸�Ӿ�����б��Σ���������Ӱ��ͼ�п��������λ���и�����������Դ�����߷ֱ���
//ʵ��: ��ʱ��ʵ��
void PerspectiveWarpingShadowMapExample(){}


//������Ӱ��ͼʾ��
//ԭ��: �������е�������ݺ����֮��ľ��뻮��Ϊ��ͬ����Ȳ㼶����Ϊÿ���㼶������Ӱ��ͼ�����������ڿ�������ĵط��ṩ���ߵķֱ���
//����: ����ÿ���㼶��û����������������Ϣ�������ڿ缶�������ͼ����ܻ�ȱ������֮����ڵ���Ϣ����������
//ʵ��: ��ʱ��ʵ��
void CascadeShadowMapExample(){}


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

const float s_pi = 3.141592653;

//˫���Բ�ֵ
vec4 BilinearInterpolate(vec4 f00, vec4 f10, vec4 f01, vec4 f11, float u, float v) {
    return mix(
        mix(f00, f10, u), // �� x �����ֵ
        mix(f01, f11, u), // �� x �����ֵ
        v                 // �� y �����ֵ
    );
}

//PCF  �ٷֱȽӽ��˲���Ӱ��ͼ
//ԭ��: ����Ƭ������Ӱ��ͼ�ж�Ӧλ�õ�һ����Χ�ڵ������Ƿ��ڵ����жϵ�ǰƬ�ε�����Ӱ���
void PercentageCloserFilteringShadowMapExample(){
	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);

	uint layer = 0;
	vec2 uv;
	
	//����ǰƬ��ת����Դ�ı任�����м������Ӧ���
	GetMatrixIndex(lightViewDir,layer,uv);
	
	//���㵱ǰƬ�����ڱ�������
	vec4 realFragPosInLightView = pointLightTransformM[layer] * vec4(inPosition,1.0);
	realFragPosInLightView /=realFragPosInLightView.w;
	float realFragDepthInLightView = realFragPosInLightView.z;


	vec3 depthTextureSize = textureSize(pointLightDepthTextureArray,0);
	vec2 deltaUV = vec2(1) / vec2(depthTextureSize.xy);


	
	
	float totalVisibility =0;

	uint filterWH = 2;//����4x4������



	for(uint i = 0;i < filterWH;i++)
	{
		for(uint j = 0;j < filterWH;j++)
		{
			vec4 visibility;
			vec2 sampleUV = uv - deltaUV * vec2(i,j);

			//�����ĸ���
			vec4 fourSampleX = textureGather(pointLightDepthTextureArray,vec3(sampleUV,layer));
			for(uint pid = 0;pid < 4;pid++)
			{
				
				if(fourSampleX[pid] + 0.0001 < realFragDepthInLightView)//����һ��ƫ��
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
	outColor = vec4(totalVisibility,totalVisibility,totalVisibility,1);

}


//����ע�͵���ֱ��ͨ���������������ȡ���ֵ�������������������ֵ�ж��Ƿ�ɼ���Ȼ����ݿɼ��ԣ�0��1����ƽ���������Ƭ���Ƿ�ɼ�������ÿ������ֻ��0��1��û�в�ֵ���������յõ��Ľ������ɢ����Ӱ��������������
//void PercentageCloserFilteringShadowMapExample(){
//	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
//	vec3 wo = normalize(inPosition -  cameraPos);
//	
//	uint layer = 0;
//	vec2 uv;
//	GetMatrixIndex(lightViewDir,layer,uv);
//	vec4 realFragPosInLightView = pointLightTransformM[layer] * vec4(inPosition,1.0);//���㵱ǰƬ�����ڱ������ȣ���Ϊ�������������ȵ��ڸ�Ƭ�ε����
//	realFragPosInLightView /=realFragPosInLightView.w;
//	float realFragDepthInLightView = realFragPosInLightView.z;
//
//
//	
//	vec3 y = -normalize(lightViewDir);
//	vec3 x,z;
//	if((1 - abs(y.x)) > 0.00000001)
//	{
//		x = vec3(1,0,0);
//	}else {
//		x = vec3(0,1,0);
//	}
//	z = normalize(cross(x,y));
//	x = normalize(cross(y,z));
//	mat3 normalMatrix = mat3(x,y,z);
//	mat3 normalMatrixInverse  = inverse(normalMatrix);
//	
//	
//	
//	//����
//	uint numSample = 8;//ÿ���ӽǵ������Ϊ4 * pi / 6;
//
//
//	float deltaFine = 0.05;
//	vec3 totalColor = vec3(0,0,0);
//
//	//����ʹ�õ�����������ͼ��ʹ��texelFetch������PCF��Ҫ�ֶ�������������ͼ�Ķ�ά�������꣬��������Ϊ�˼�ֱ��ͨ�����ݲ�������ƫ��һ���Ƕ�������������ķ�ʽ����
//	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
//	{
//		vec2 samplePoint = HaltonSample2D(sampleIndex);
//		float theta = samplePoint.x * 2 * s_pi;
//		float fine = (samplePoint.y-0.5) * 2 * deltaFine;
//
//		//��������ת��Ϊ����������ת������������ϵ��
//		vec3 sampleV;
//		sampleV.y = -cos(fine);
//		sampleV.x = sin(fine) * cos(theta);
//		sampleV.z = sin(fine) * sin(theta);
//		sampleV = normalize(normalMatrix * sampleV);
//
//		//����
//		float curLightDirMinFragSampleDepth = texture(pointLightDepthTexture,sampleV).x;//��������ͼ�е����
//
//
//		if(curLightDirMinFragSampleDepth + 0.000001 >= realFragDepthInLightView)//����һ��ƫ�ƺ�������������С��Ƭ������ƽ�����ȣ���˵����������������Ӱ��
//		{
//			totalColor +=vec3(1,1,1) ;
//			
//		}else {
//			float tmp =1 - abs((samplePoint.y-0.5)* 2); 
//			totalColor +=vec3(0,0,0);//����Ӱ�о�Ϊ��ɫ
//			//totalColor +=vec3(tmp,tmp,tmp);//����Ӱ�о�Ϊ��ɫ
//		}
//	
//	}
//	totalColor/=numSample;
//	outColor = vec4(totalColor,1);
//
//}


//PCSS
//ԭ��: PCF�ĸ����棬����ͨ����Դ���ڵ����Լ���������֮��ľ����ϵ����̬ȷ����������Ĵ�С���Դ���ȷ������Ӱ�ĳ̶ȣ������������СΪ w * (dr - d0) / d0,����wΪ��ʼ�����С��drΪ����Դ����������֮��ľ��룬d0Ϊ��Դ���ڵ���֮��ľ���

void PercentageCloserSoftShadowMapExample(){
	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);

	uint layer = 0;
	vec2 uv;
	
	//����ǰƬ��ת����Դ�ı任�����м������Ӧ���
	GetMatrixIndex(lightViewDir,layer,uv);
	
	//���㵱ǰƬ�����ڱ�������
	vec4 realFragPosInLightView = pointLightTransformM[layer] * vec4(inPosition,1.0);
	realFragPosInLightView /=realFragPosInLightView.w;
	float realFragDepthInLightView = realFragPosInLightView.z;


	vec3 depthTextureSize = textureSize(pointLightDepthTextureArray,0);
	vec2 deltaUV = vec2(1) / vec2(depthTextureSize.xy);


	float curLightDirMinFragDepth = texture(pointLightDepthTexture,lightViewDir).x;//ֱ��ʹ��cube��������������ڵ����ƽ������
	float curLightDirMinFragRealDepth = texture(pointLightDepthTexture,lightViewDir).y;

	//��uvת��Ϊ�ü������x����y
	vec2 clipXY = uv * 2 - 1;
	vec3 clipXYZ = vec3(clipXY,curLightDirMinFragDepth);

	mat4 inverseVP = inverse(pointLightTransformM[layer]);

	//�����ڵ�������ռ��е�λ��
	vec3 occluderPos = vec4(inverseVP * vec4(clipXYZ,1.0)).xyz;

	//�����Դ���ڵ���֮��ľ��� 
	float dloccluder = distance(pointLightPos,occluderPos);
	
	//�����Դ�ͽ�������֮��ľ��� 
	float dlobject = distance(pointLightPos,inPosition);


	float totalVisibility =0;

	uint filterWH = 8;//����6x6������


	//���ݾ��붯̬���������С
	//float d0 = curLightDirMinFragDepth;
	//float dr =  realFragDepthInLightView;
	float d0 = dloccluder;
	float dr =  dlobject;
	float ratio = (dr - d0) / d0;
	float w = filterWH * ratio;
	filterWH = clamp(uint(w),2,2*filterWH);


	for(uint i = 0;i < filterWH;i++)
	{
		for(uint j = 0;j < filterWH;j++)
		{
			vec4 visibility;
			vec2 sampleUV = uv - deltaUV * vec2(i,j);

			//�����ĸ���
			vec4 fourSampleX = textureGather(pointLightDepthTextureArray,vec3(sampleUV,layer));
			for(uint pid = 0;pid < 4;pid++)
			{
				
				if(fourSampleX[pid] + 0.0001 < realFragDepthInLightView)//����һ��ƫ��
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
	outColor = vec4(totalVisibility,totalVisibility,totalVisibility,1);
}

//VSM ������Ӱ��ͼ
//ԭ��:

void VarianceShadowMapExample(){
	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);

	uint layer = 0;
	vec2 uv;
	
	//����ǰƬ��ת����Դ�ı任�����м������Ӧ���
	GetMatrixIndex(lightViewDir,layer,uv);
	
	vec3 depthTextureSize = textureSize(pointLightDepthTextureArray,0);
	vec2 deltaUV = vec2(1) / vec2(depthTextureSize.xy);


	float curLightDirMinFragDepth = texture(pointLightDepthTexture,lightViewDir).x;//ֱ��ʹ��cube��������������ڵ����ƽ������
	float curLightDirMinFragRealDepth = texture(pointLightDepthTexture,lightViewDir).y;//�������ͼ�õ����Ǹô���ƽ�����
	float curLightDirMinFragRealDepth2 = texture(pointLightDepthTexture,lightViewDir).z;


	//�����Դ�ͽ�������֮��ľ��� 
	float dLightObject = distance(pointLightPos,inPosition);

	float visibility = 1;
	float sigma2  = curLightDirMinFragRealDepth2 - pow(curLightDirMinFragRealDepth,2);//���㷽��
	float variance2 = pow(dLightObject - curLightDirMinFragRealDepth,2);

	if(curLightDirMinFragRealDepth + 0.0001 < dLightObject)//˵������Ӱ��
	{
		
		visibility = sigma2 / (sigma2 +  variance2);
	
	}

	outColor = vec4(visibility,visibility,visibility,1.0);

}

//��͸����Ӱ��ͼ -  �����Ӱ����

void DeepShadowMapExample(){}


//������z-buffer 
//ԭ��: ��Ӱ��ͼ��ÿ��texel��洢������߶�����������λ�ã��⵼����Ӱ��ͼ��ʵ�������ǲ�����ģ�ͨ�����ַ�ʽ����ʵ��
//��������в�����Ӱ��ͼ��ʱ��Ϳ��Ա����������Ϣ������ȷ�����������Ƿ�����Ӱ���Լ�������Ϣ��
void IrregularZBufferExample(){}

void main(){
	//if(inPosition.y >=0){
	//	outColor = vec4(1,1,1,1);
	//}else if(inPosition.y >-1){
	//	outColor = vec4(0,1,0,1);
	//}else {
	//outColor = vec4(1,0,0,1);
	//}
	outColor = vec4(1,1,1,1);
	if(exampleType == 0)
	{
		ShadowMapExample();
	
	}else if(exampleType == 1)
	{
		PercentageCloserFilteringShadowMapExample();
	
	}else if(exampleType == 2){
		PercentageCloserSoftShadowMapExample();
	}else if(exampleType == 3)
	{
		VarianceShadowMapExample();
	}


	//outColor = vec4(1,1,1,1);
	//ShadowMapExample();
	//PercentageCloserFilteringShadowMapExample();
	//PercentageCloserSoftShadowMapExample();
	//VarianceShadowMapExample();
}