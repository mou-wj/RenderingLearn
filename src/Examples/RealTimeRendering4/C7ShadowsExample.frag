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


};

//平面投影阴影，主要原理为将遮挡体通过投影矩阵的方式计算出再平面上的投影点，然后投影点在变换在平面正常的渲染中处理，可以通过几何着色器生成投影点组成的三角形等方式，或者使用纹理以及其他方式，这里不展开描述
void PlaneCastShadowExample(){}

//阴影体阴影，主要原理为每个三角形根据光源方向可以有一个阴影体，如果从眼睛中发射出的光最终穿过这个阴影体到达指定位置，这说明这个位置不在阴影中，反之则在阴影中
void ShadowVolumeExample(){}

uint GetMatrixIndex(vec3 sampleV){
	float absx = abs(sampleV.x);
	float absy = abs(sampleV.y);
	float absz = abs(sampleV.z);
	if(absx >=absy && absx >= absz)
	{
		if(sampleV.x >=0){
			return 0;
		}else {
			return 1;
		}
	
	}

	if(absy >=absx && absy >= absz)
	{
		if(sampleV.y >=0){
			return 2;
		}else {
			return 3;
		}
	
	}

	if(absz >=absx && absz >= absy)
	{
		if(sampleV.z >=0){
			return 4;
		}else {
			return 5;
		}
	
	}

	return 0;
}

//阴影贴图阴影，主要是用过额外的阴影贴图来判断某一个位置是否在阴影中
void ShadowMapExample(){
	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);
	float curLightDirMinFragDepth = texture(pointLightDepthTexture,lightViewDir).x;
	//将当前片段转到光源的变换矩阵中计算其对应深度
	uint transMatrixIndex= GetMatrixIndex(lightViewDir);
	vec4 realFragPosInLightView = pointLightTransformM[transMatrixIndex] * vec4(inPosition,1.0);
	realFragPosInLightView /=realFragPosInLightView.w;
	float realFragDepthInLightView = realFragPosInLightView.z;

	if(curLightDirMinFragDepth < realFragDepthInLightView)
	{
		outColor = vec4(0,0,1,1);
	
	}




}

void main(){
	//if(inPosition.y >=0){
	//	outColor = vec4(1,1,1,1);
	//}else if(inPosition.y >-1){
	//	outColor = vec4(0,1,0,1);
	//}else {
	//outColor = vec4(1,0,0,1);
	//}
	outColor = vec4(1,1,1,1);
	//outColor = vec4(1,1,1,1);
	ShadowMapExample();
}