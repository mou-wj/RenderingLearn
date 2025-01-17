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


};

//ƽ��ͶӰ��Ӱ����Ҫԭ��Ϊ���ڵ���ͨ��ͶӰ����ķ�ʽ�������ƽ���ϵ�ͶӰ�㣬Ȼ��ͶӰ���ڱ任��ƽ����������Ⱦ�д�������ͨ��������ɫ������ͶӰ����ɵ������εȷ�ʽ������ʹ�������Լ�������ʽ�����ﲻչ������
void PlaneCastShadowExample(){}

//��Ӱ����Ӱ����Ҫԭ��Ϊÿ�������θ��ݹ�Դ���������һ����Ӱ�壬������۾��з�����Ĺ����մ��������Ӱ�嵽��ָ��λ�ã���˵�����λ�ò�����Ӱ�У���֮������Ӱ��
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

//��Ӱ��ͼ��Ӱ����Ҫ���ù��������Ӱ��ͼ���ж�ĳһ��λ���Ƿ�����Ӱ��
void ShadowMapExample(){
	vec3 lightViewDir = normalize(inPosition -  pointLightPos);
	vec3 wo = normalize(inPosition -  cameraPos);
	float curLightDirMinFragDepth = texture(pointLightDepthTexture,lightViewDir).x;
	//����ǰƬ��ת����Դ�ı任�����м������Ӧ���
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