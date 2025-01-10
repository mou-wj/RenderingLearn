#version 450 core


layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��е�λ��
layout(location = 0) out vec4 outColor;
layout(set = 0,binding = 1,std140) uniform SceenInfoBuffer{
	vec3 cameraPos;
	layout(offset = 16) vec3 PointLightPos;
	layout(offset = 32) vec3 PointLightColor;
};

//��Goochģ�ͣ�ֻ��һ�����Դ�����巽�򣬲�����˥��
vec3 GoochShading(){

	vec3 res;


	vec3 wo = normalize(cameraPos - inPosition);
	vec3 l = normalize(PointLightPos - inPosition);
	float nDotL =  max(dot(inNormal,l),0);

	vec3 r = reflect(-l,inNormal);
	float rDotWo = max(dot(r,wo),0);

	vec3 cSurface = vec3(0.1,0.1,0.1);//����Թ�ķ�Ӧ



	
	vec3 cHighlight = vec3(1,1,1);
	vec3 cWarm = vec3(0.3,0.3,0) + 0.25 * cSurface;
	vec3 cCool = vec3(0,0,0.3) + 0.25*cSurface;
	float t = (nDotL + 1)/2;
	float s = clamp(100 * rDotWo - 97,0,1);

	res = s * cHighlight + (1-s) * (t * cWarm + (1-t) * cCool);


	return res;
}

//Goochģ�͵���չ�����Դ
vec3 GoochShadingExtend(){
	vec3 res;
	vec3 cSurface = vec3(0.1,0.1,0.1);//����Թ�ķ�Ӧ
	vec3 cCool = vec3(0,0,0.55) + 0.25*cSurface;
	vec3 cWarm = vec3(0.3,0.3,0) + 0.25 * cSurface;
	vec3 cHighlight = vec3(2,2,2);
		
	vec3 wo = normalize(cameraPos - inPosition);
	vec3 l = normalize(PointLightPos - inPosition);
	vec3 r = normalize(reflect(-l,inNormal));
	float rDotWo = max(dot(r,wo),0);
	float nDotL =  max(dot(inNormal,l),0);

	float s = clamp(100 * rDotWo - 97,0,1);





	float maxDist = 10;//�������˥������
	//����
	float dist = distance(PointLightPos,inPosition);

	float fwin = pow(clamp(1 - pow(dist / maxDist,4) ,0,1),2);//��������˥������Ĵ�����
	float attenationF = 1/(pow(dist,2)+ 0.01) * fwin;//�ɴ��������Ƶ�˥������

	vec3 recivedLight = PointLightColor * attenationF;//�����˥���ĵ��Դ


	res = 0.5 * cCool + nDotL * recivedLight * ( s * cHighlight + (1-s ) * cWarm);
	
	//res = recivedLight;
	//res = vec3(nDotL,0,0);
	return res;
}


void main(){

//�������ж������Ϊ��ɫ
	outColor = vec4(1,1,1,1);
	outColor = vec4(GoochShading(),1.0);
	//outColor = vec4(GoochShadingExtend(),1.0);
}