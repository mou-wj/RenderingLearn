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

//ƽ��ͶӰ��Ӱ����Ҫԭ��Ϊ���ڵ���ͨ��ͶӰ����ķ�ʽ�������ƽ���ϵ�ͶӰ�㣬Ȼ��ͶӰ���ڱ任��ƽ����������Ⱦ�д�������ͨ��������ɫ������ͶӰ����ɵ������εȷ�ʽ������ʹ�������Լ�������ʽ�����ﲻչ������
void PlaneCastShadowExample(){}

//��Ӱ����Ӱ����Ҫԭ��Ϊÿ�������θ��ݹ�Դ���������һ����Ӱ�壬������۾��з�����Ĺ����մ��������Ӱ�嵽��ָ��λ�ã���˵�����λ�ò�����Ӱ�У���֮������Ӱ��
void ShadowVolumeExample(){}


//��Ӱ��ͼ��Ӱ����Ҫ���ù��������Ӱ��ͼ���ж�ĳһ��λ���Ƿ�����Ӱ��
void ShadowMapExample(){


}

void main(){
	outColor = vec4(1,1,1,1);

}