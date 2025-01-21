#version 450 core

//遗留问题：
//SAT采样返回全0的问题
//法线贴图正确性验证
//视差贴图
//纹理光源

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的位置
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;


layout(set = 0,binding = 1) uniform samplerCube pointLightDepthTexture;
layout(set = 0,binding = 2,std140) uniform SceenInfo{
	vec3 pointLightPos;
	layout(offset = 16) vec3 cameraPos;
	layout(offset = 32) mat4 pointLightTransformM[6];//分别为+x,-x,+y,-y,+z,-z六个方向的MVP变换矩阵
	uint exampleType;//表示要显示的阴影算法示例。0:表示直接通过阴影贴图; 1:PCF 百分比滤波; 2: PCSS -PCF改进版本，根据距离信息控制滤波核大小; 3:CVM 方差阴影贴图 

};
layout(set = 0,binding = 3) uniform sampler2DArray pointLightDepthTextureArray;


//平面投影阴影，主要原理为将遮挡体通过投影矩阵的方式计算出再平面上的投影点，然后投影点在变换在平面正常的渲染中处理，可以通过几何着色器生成投影点组成的三角形等方式，或者使用纹理以及其他方式，这里不展开描述
void PlaneCastShadowExample(){}

//阴影体阴影，主要原理为每个三角形根据光源方向可以有一个阴影体，如果从眼睛中发射出的光最终穿过这个阴影体到达指定位置，这说明这个位置不在阴影中，反之则在阴影中
void ShadowVolumeExample(){}

void GetMatrixIndex(vec3 sampleV,out uint layer,out vec2 uv){
	float absx = abs(sampleV.x);
	float absy = abs(sampleV.y);
	float absz = abs(sampleV.z);
	float u = 0,v = 0;
	if(absx >=absy && absx >= absz)
	{

		if(sampleV.x >=0){
			//手动计算uv
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
	//v = -v;//翻转v
	uv = vec2(u,v);

}

//阴影贴图阴影，主要是用过额外的阴影贴图来判断某一个位置是否在阴影中，硬阴影
void ShadowMapExample(){

	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);
	float curLightDirMinFragDepth = texture(pointLightDepthTexture,lightViewDir).x;//直接使用cube纹理采样

	uint layer = 0;
	vec2 uv;
	
	//将当前片段转到光源的变换矩阵中计算其对应深度
	GetMatrixIndex(lightViewDir,layer,uv);
	vec2 curuv = gl_FragCoord.xy / vec2(512,512);

	vec3 depthTextureSize = textureSize(pointLightDepthTextureArray,0);
	//float s = texelFetch(pointLightDepthTexture,ivec3(ivec2(uv),layer));
	float curLightDirMinFragDepthUV = texelFetch(pointLightDepthTextureArray,ivec3(ivec2(uv * depthTextureSize.xy),layer),0).x;

	vec4 realFragPosInLightView = pointLightTransformM[layer] * vec4(inPosition,1.0);
	realFragPosInLightView /=realFragPosInLightView.w;
	float realFragDepthInLightView = realFragPosInLightView.z;

	if(curLightDirMinFragDepthUV + 0.0001 < realFragDepthInLightView)//加上一定偏移
	{
		outColor = vec4(0,0,0,1);
	
	}




}

//透视走样阴影贴图
//原理: 根据场景中的物体和相机之间的距离，对生成阴影贴图的透视矩阵进行变形，用来让阴影贴图中靠近相机的位置有更多的样本，以此来提高分辨率
//实现: 暂时不实现
void PerspectiveWarpingShadowMapExample(){}


//级联阴影贴图示例
//原理: 将场景中的物体根据和相机之间的距离划分为不同的深度层级，并为每个层级生成阴影贴图，这样可以在靠近相机的地方提供更高的分辨率
//问题: 由于每个层级并没有所有物体的深度信息，所以在跨级的深度贴图间可能会缺少物体之间的遮挡信息，导致问题
//实现: 暂时不实现
void CascadeShadowMapExample(){}


//生成随机数
// 生成Hammersley 序列的一维采样点，各个维度的序列可以使用不同的基来生成
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

// 生成均匀分布的二维Halton采样点
vec2 HaltonSample2D(uint index) {
    return vec2(HammersleySequence(index, 2),HammersleySequence(index, 3)); // 基数选择 2,3
}

const float s_pi = 3.141592653;

//双线性插值
vec4 BilinearInterpolate(vec4 f00, vec4 f10, vec4 f01, vec4 f11, float u, float v) {
    return mix(
        mix(f00, f10, u), // 在 x 方向插值
        mix(f01, f11, u), // 在 x 方向插值
        v                 // 在 y 方向插值
    );
}

//PCF  百分比接近滤波阴影贴图
//原理: 根据片段在阴影贴图中对应位置的一定范围内的样本是否被遮挡来判断当前片段的软阴影层度
void PercentageCloserFilteringShadowMapExample(){
	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);

	uint layer = 0;
	vec2 uv;
	
	//将当前片段转到光源的变换矩阵中计算其对应深度
	GetMatrixIndex(lightViewDir,layer,uv);
	
	//计算当前片段所在表秒的深度
	vec4 realFragPosInLightView = pointLightTransformM[layer] * vec4(inPosition,1.0);
	realFragPosInLightView /=realFragPosInLightView.w;
	float realFragDepthInLightView = realFragPosInLightView.z;


	vec3 depthTextureSize = textureSize(pointLightDepthTextureArray,0);
	vec2 deltaUV = vec2(1) / vec2(depthTextureSize.xy);


	
	
	float totalVisibility =0;

	uint filterWH = 2;//定义4x4的网格



	for(uint i = 0;i < filterWH;i++)
	{
		for(uint j = 0;j < filterWH;j++)
		{
			vec4 visibility;
			vec2 sampleUV = uv - deltaUV * vec2(i,j);

			//采样四个点
			vec4 fourSampleX = textureGather(pointLightDepthTextureArray,vec3(sampleUV,layer));
			for(uint pid = 0;pid < 4;pid++)
			{
				
				if(fourSampleX[pid] + 0.0001 < realFragDepthInLightView)//加上一定偏移
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
	outColor = vec4(totalVisibility,totalVisibility,totalVisibility,1);

}


//以下注释的是直接通过采样随机向量获取深度值，并对所有样本的深度值判断是否可见，然后根据可见性（0，1）的平均来计算该片段是否可见，由于每个样本只有0，1，没有插值，所以最终得到的结果是离散的阴影，看起来不连续
//void PercentageCloserFilteringShadowMapExample(){
//	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
//	vec3 wo = normalize(inPosition -  cameraPos);
//	
//	uint layer = 0;
//	vec2 uv;
//	GetMatrixIndex(lightViewDir,layer,uv);
//	vec4 realFragPosInLightView = pointLightTransformM[layer] * vec4(inPosition,1.0);//计算当前片段所在表面的深度，认为所有样本点的深度等于该片段的深度
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
//	//采样
//	uint numSample = 8;//每个视角的立体角为4 * pi / 6;
//
//
//	float deltaFine = 0.05;
//	vec3 totalColor = vec3(0,0,0);
//
//	//由于使用的是立方体贴图，使用texelFetch来计算PCF需要手动计算立方体贴图的二维纹理坐标，所以这里为了简单直接通过根据采样向量偏移一定角度来构建采样点的方式进行
//	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
//	{
//		vec2 samplePoint = HaltonSample2D(sampleIndex);
//		float theta = samplePoint.x * 2 * s_pi;
//		float fine = (samplePoint.y-0.5) * 2 * deltaFine;
//
//		//将样本点转换为采样向量并转换到世界坐标系中
//		vec3 sampleV;
//		sampleV.y = -cos(fine);
//		sampleV.x = sin(fine) * cos(theta);
//		sampleV.z = sin(fine) * sin(theta);
//		sampleV = normalize(normalMatrix * sampleV);
//
//		//采样
//		float curLightDirMinFragSampleDepth = texture(pointLightDepthTexture,sampleV).x;//采样纹理图中的深度
//
//
//		if(curLightDirMinFragSampleDepth + 0.000001 >= realFragDepthInLightView)//加上一定偏移后如果采样的深度小于片段所在平面的深度，则说明该样本根本在阴影中
//		{
//			totalColor +=vec3(1,1,1) ;
//			
//		}else {
//			float tmp =1 - abs((samplePoint.y-0.5)* 2); 
//			totalColor +=vec3(0,0,0);//在阴影中就为黑色
//			//totalColor +=vec3(tmp,tmp,tmp);//在阴影中就为黑色
//		}
//	
//	}
//	totalColor/=numSample;
//	outColor = vec4(totalColor,1);
//
//}


//PCSS
//原理: PCF的改良版，可以通过光源，遮挡物以及接受物体之间的距离关系来动态确定采样网格的大小，以此来确定软阴影的程度，即采样网格大小为 w * (dr - d0) / d0,其中w为初始网格大小，dr为到光源到接受物体之间的距离，d0为光源到遮挡物之间的距离

void PercentageCloserSoftShadowMapExample(){
	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);

	uint layer = 0;
	vec2 uv;
	
	//将当前片段转到光源的变换矩阵中计算其对应深度
	GetMatrixIndex(lightViewDir,layer,uv);
	
	//计算当前片段所在表秒的深度
	vec4 realFragPosInLightView = pointLightTransformM[layer] * vec4(inPosition,1.0);
	realFragPosInLightView /=realFragPosInLightView.w;
	float realFragDepthInLightView = realFragPosInLightView.z;


	vec3 depthTextureSize = textureSize(pointLightDepthTextureArray,0);
	vec2 deltaUV = vec2(1) / vec2(depthTextureSize.xy);


	float curLightDirMinFragDepth = texture(pointLightDepthTexture,lightViewDir).x;//直接使用cube纹理采样来决定遮挡物的平均距离
	float curLightDirMinFragRealDepth = texture(pointLightDepthTexture,lightViewDir).y;

	//将uv转换为裁剪坐标的x，和y
	vec2 clipXY = uv * 2 - 1;
	vec3 clipXYZ = vec3(clipXY,curLightDirMinFragDepth);

	mat4 inverseVP = inverse(pointLightTransformM[layer]);

	//计算遮挡在世界空间中的位置
	vec3 occluderPos = vec4(inverseVP * vec4(clipXYZ,1.0)).xyz;

	//计算光源和遮挡物之间的距离 
	float dloccluder = distance(pointLightPos,occluderPos);
	
	//计算光源和接受物体之间的距离 
	float dlobject = distance(pointLightPos,inPosition);


	float totalVisibility =0;

	uint filterWH = 8;//定义6x6的网格


	//根据距离动态调整网格大小
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

			//采样四个点
			vec4 fourSampleX = textureGather(pointLightDepthTextureArray,vec3(sampleUV,layer));
			for(uint pid = 0;pid < 4;pid++)
			{
				
				if(fourSampleX[pid] + 0.0001 < realFragDepthInLightView)//加上一定偏移
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
	outColor = vec4(totalVisibility,totalVisibility,totalVisibility,1);
}

//VSM 方差阴影贴图
//原理:

void VarianceShadowMapExample(){
	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);

	uint layer = 0;
	vec2 uv;
	
	//将当前片段转到光源的变换矩阵中计算其对应深度
	GetMatrixIndex(lightViewDir,layer,uv);
	
	vec3 depthTextureSize = textureSize(pointLightDepthTextureArray,0);
	vec2 deltaUV = vec2(1) / vec2(depthTextureSize.xy);


	float curLightDirMinFragDepth = texture(pointLightDepthTexture,lightViewDir).x;//直接使用cube纹理采样来决定遮挡物的平均距离
	float curLightDirMinFragRealDepth = texture(pointLightDepthTexture,lightViewDir).y;//假设该贴图得到的是该处的平均深度
	float curLightDirMinFragRealDepth2 = texture(pointLightDepthTexture,lightViewDir).z;


	//计算光源和接受物体之间的距离 
	float dLightObject = distance(pointLightPos,inPosition);

	float visibility = 1;
	float sigma2  = curLightDirMinFragRealDepth2 - pow(curLightDirMinFragRealDepth,2);//计算方差
	float variance2 = pow(dLightObject - curLightDirMinFragRealDepth,2);

	if(curLightDirMinFragRealDepth + 0.0001 < dLightObject)//说明在阴影中
	{
		
		visibility = sigma2 / (sigma2 +  variance2);
	
	}

	outColor = vec4(visibility,visibility,visibility,1.0);

}

//不透明阴影贴图 -  体积阴影技术

void DeepShadowMapExample(){}


//不规则z-buffer 
//原理: 阴影贴图中每个texel会存储零个或者多个接受物体的位置，这导致阴影贴图的实际内容是不规则的，通过这种方式，在实际
//计算过程中查找阴影贴图的时候就可以遍历额外的信息，最终确定接受物体是否在阴影中以及其他信息等
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