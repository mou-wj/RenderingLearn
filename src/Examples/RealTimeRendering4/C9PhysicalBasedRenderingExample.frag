#version 450 core

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的位置
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;


layout(set = 0,binding = 3,std140) uniform Info{
	float animateDelta;
	layout(offset = 16) vec3 cameraPos;//世界空间中相机位置
	layout(offset = 32) int exampleType;//要显示的实例类型，0: 直接获取纹素值 ; 1:mipmap ;2: SAT; 3:随机生成的纹理;4:纹理动画;5:材质纹理;6:alpha贴图
};

vec3 Frenel(float nDotL,float eta/*相对折射率，入射的介质折射率/入射光所在的介质折射率 */){
	float F0 = pow((eta - 1)/(eta + 1),2); 
	float r = F0 + (1- F0) * pow(1 - max(nDotL,0),5);
	return vec3(r);

}




void main(){
	outColor = vec4(1,0,0,1);

}