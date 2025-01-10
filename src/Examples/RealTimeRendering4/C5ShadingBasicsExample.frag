#version 450 core


layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的位置
layout(location = 0) out vec4 outColor;
layout(set = 0,binding = 1,std140) uniform SceenInfoBuffer{
	vec3 cameraPos;
	layout(offset = 16) vec3 PointLightPos;
	layout(offset = 32) vec3 PointLightColor;
};

//简单Gooch模型，只用一个点光源来定义方向，不考虑衰减
vec3 GoochShading(){

	vec3 res;


	vec3 wo = normalize(cameraPos - inPosition);
	vec3 l = normalize(PointLightPos - inPosition);
	float nDotL =  max(dot(inNormal,l),0);

	vec3 r = reflect(-l,inNormal);
	float rDotWo = max(dot(r,wo),0);

	vec3 cSurface = vec3(0.1,0.1,0.1);//表面对光的反应



	
	vec3 cHighlight = vec3(1,1,1);
	vec3 cWarm = vec3(0.3,0.3,0) + 0.25 * cSurface;
	vec3 cCool = vec3(0,0,0.3) + 0.25*cSurface;
	float t = (nDotL + 1)/2;
	float s = clamp(100 * rDotWo - 97,0,1);

	res = s * cHighlight + (1-s) * (t * cWarm + (1-t) * cCool);


	return res;
}

//Gooch模型的拓展，多光源
vec3 GoochShadingExtend(){
	vec3 res;
	vec3 cSurface = vec3(0.1,0.1,0.1);//表面对光的反应
	vec3 cCool = vec3(0,0,0.55) + 0.25*cSurface;
	vec3 cWarm = vec3(0.3,0.3,0) + 0.25 * cSurface;
	vec3 cHighlight = vec3(2,2,2);
		
	vec3 wo = normalize(cameraPos - inPosition);
	vec3 l = normalize(PointLightPos - inPosition);
	vec3 r = normalize(reflect(-l,inNormal));
	float rDotWo = max(dot(r,wo),0);
	float nDotL =  max(dot(inNormal,l),0);

	float s = clamp(100 * rDotWo - 97,0,1);





	float maxDist = 10;//定义最大衰减距离
	//距离
	float dist = distance(PointLightPos,inPosition);

	float fwin = pow(clamp(1 - pow(dist / maxDist,4) ,0,1),2);//控制有限衰减距离的窗函数
	float attenationF = 1/(pow(dist,2)+ 0.01) * fwin;//由窗函数控制的衰减因子

	vec3 recivedLight = PointLightColor * attenationF;//添加了衰减的点光源


	res = 0.5 * cCool + nDotL * recivedLight * ( s * cHighlight + (1-s ) * cWarm);
	
	//res = recivedLight;
	//res = vec3(nDotL,0,0);
	return res;
}


void main(){

//场景所有对象绘制为白色
	outColor = vec4(1,1,1,1);
	outColor = vec4(GoochShading(),1.0);
	//outColor = vec4(GoochShadingExtend(),1.0);
}