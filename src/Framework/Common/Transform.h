#pragma once


#include <glm/matrix.hpp>

//���±任������ϵ��

namespace Transform {

	/**
 * @brief �����ת������ת��������ϵ������Ĵָָ����ת�᷽������ָ�����ķ�����ת
 * @param degree �Ƕȣ��Ƕ�ֵ���ǻ��ȣ�
 * @param rotateAxis ��ת��������������ת����Χ�Ƶ�����
 * @return ������ת����
*/
	glm::mat4 GetRotateMatrix(float degree, glm::vec3 rotateAxis);

	/**
	 * @brief ������ž���
	 * @param scaleVec ������������ʾx��y��z���������ŵĴ�С
	 * @return ������������
	*/
	glm::mat4 GetScaleMatrix(glm::vec3 scaleVec);

	/**
	 * @brief ���λ�ƾ���
	 * @param translateVec λ����������ʾ����x��y��z������λ�ƴ�С������
	 * @return ����λ�ƾ���
	*/
	glm::mat4 GetTranslateMatrix(glm::vec3 translateVec);

	/**
	 * @brief ��������
	 * @param srcMatrix Դ����
	 * @return ����Դ����������
	*/
	glm::mat4 GetInverseMatrix(glm::mat4 srcMatrix);

	/**
	 * @brief ���ת�þ���
	 * @param srcMatrix Դ����
	 * @return ����Դ�����ת�þ���
	*/
	glm::mat4 GetTransposeMatrix(glm::mat4 srcMatrix);

	/**
	 * @brief ���ŷ����ת���󣬳�ʼ����-z������ϵ
	 * @param yaw ƫ���ǣ��Ƕ��ƣ�������ҿ�,��y����ת��x-z,Ϊ����ʾvulkan��NDC������
	 * @param pitch ������ ���Ƕ��ƣ�������¿�����x����ת,y-x,Ϊ����ʾvulkan��NDC�����Ͽ�
	 * @param roll ������ ���Ƕ��ƣ����˳ʱ�뿴����z����ת,x-y��Ϊ����ʾvulkan��NDC��˳ʱ������ת,�ȼ���ͼ������ʱ��ת
	 * @return ����ŷ����ת����
	*/
	glm::mat4 GetEularRotateMatrix(float yaw, float pitch, float roll);

	/**
	 * @brief �����Ԫ����ת���� ��Ԫ�� q ���Կ����� q = �� sin(f)* u, cos(f) �� = sin(f) * u + cos(f) = exp(f * u)������fΪ��ά�ռ�ĵ�һ����λ������fΪһ���Ƕȣ�
	 *		  ����������Real-Time Renderering 4th page-80  ��Ԫ�������ʾ�ڿռ��� ��Ĵָָ��u��������ָ�����ķ�����ת2f�ĽǶ� ����ϵ
	 * @param center ��ת�����Ԫ������vec3(u) * sin(f),cos(f)��
	 * @return ������Ԫ����ת����
	*/
	glm::mat4 GetQuaternionMatrix(glm::vec4 center);

	/**
	 * @brief ����������ϵ����ȡview��������downΪ��������y����Ӧvulkan��NDC�����£�target-pos��Ӧ��������z���򣬶�Ӧvulkan��NDC���ڷ���
	 * @param pos ���λ�ã�target ����ӵ㣬down��������
	 * @return ����view����
	*/
	glm::mat4 GetViewMatrix(glm::vec3 pos,glm::vec3 target, glm::vec3 down);



	/**
	 * @brief ��ȡͶӰ���󣬲���������ϵ������z��������
	 * @param near ��ƽ�棬far Զƽ�棬viewAngle����׶����֮��ļнǣ��Ƕ�ֵ��0-180����ratioWH����׶��ƽ��Ŀ�߱�,near��far���붼����0����far����near
	 * @return ����͸��ͶӰ����,��׶��������x��y����任��-1��1֮�䣬��ƽ��Ϊ0��Զƽ��Ϊ1
	*/
	glm::mat4 GetPerspectiveProj(float near, float far, float viewAngle, float ratioWH);


	/**
	 * @brief ��ȡƽ��ͶӰ���󣬲���������ϵ������z��������
	 * @param near ��ƽ�棬far Զƽ�棬width����ƽ��Ŀ�ȣ�height����ƽ��ĸ߶�
	 * @return ����ƽ��ͶӰ����,��׶��������x��y����任��-1��1֮�䣬��ƽ��Ϊ0��Զƽ��Ϊ1
	*/
	glm::mat4 GetParallelProj(float near, float far, float width, float height);

}


