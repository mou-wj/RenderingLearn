#pragma once


#include <glm/matrix.hpp>


namespace Transform {

	/**
 * @brief �����ת������ת��������ϵ������Ĵָָ����ת�᷽������ָ�����ķ�����ת
 * @param degree �Ƕȣ��Ƕ�ֵ���ǻ��ȣ�
 * @param rotateAxis ��ת��������������ת����Χ�Ƶ�����
 * @return ������ת����
*/
	static glm::mat4 GetRotateMatrix(float degree, glm::vec3 rotateAxis);

	/**
	 * @brief ������ž���
	 * @param scaleVec ������������ʾx��y��z���������ŵĴ�С
	 * @return ������������
	*/
	static glm::mat4 GetScaleMatrix(glm::vec3 scaleVec);

	/**
	 * @brief ���λ�ƾ���
	 * @param translateVec λ����������ʾ����x��y��z������λ�ƴ�С������
	 * @return ����λ�ƾ���
	*/
	static glm::mat4 GetTranslateMatrix(glm::vec3 translateVec);

	/**
	 * @brief ��������
	 * @param srcMatrix Դ����
	 * @return ����Դ����������
	*/
	static glm::mat4 GetInverseMatrix(glm::mat4 srcMatrix);

	/**
	 * @brief ���ת�þ���
	 * @param srcMatrix Դ����
	 * @return ����Դ�����ת�þ���
	*/
	static glm::mat4 GetTransposeMatrix(glm::mat4 srcMatrix);

	/**
	 * @brief ���ŷ����ת���󣬳�ʼ����-z������ϵ
	 * @param yaw ƫ���ǣ��Ƕ��ƣ�������ҿ�
	 * @param pitch ������ ���Ƕ��ƣ�������¿�
	 * @param roll ������ ���Ƕ��ƣ����˳ʱ�뿴
	 * @return ����ŷ����ת����
	*/
	static glm::mat4 GetEularRotateMatrix(float yaw, float pitch, float roll);

	/**
	 * @brief �����Ԫ����ת���� ��Ԫ�� q ���Կ����� q = �� sin(f)* u, cos(f) �� = sin(f) * u + cos(f) = exp(f * u)������fΪ��ά�ռ�ĵ�һ����λ������fΪһ���Ƕȣ�
	 *		  ����������Real-Time Renderering 4th page-80  ��Ԫ�������ʾ�ڿռ��� ��Ĵָָ��u��������ָ�����ķ�����ת2f�ĽǶ� ����ϵ
	 * @param center ��ת�����Ԫ������vec3(u) * sin(f),cos(f)��
	 * @return ������Ԫ����ת����
	*/
	static glm::mat4 GetQuaternionMatrix(glm::vec4 center);

	/**
	 * @brief ����ϵ����ȡview��������upΪ��������-y����Ӧvulkan��NDC�����£�target-pos��Ӧ��������-z���򣬶�Ӧvulkan��NDC���ڷ���
	 * @param pos ���λ�ã�target ����ӵ㣬up��������
	 * @return ����view����
	*/
	static glm::mat4 GetViewMatrix(glm::vec3 pos,glm::vec3 target, glm::vec3 up);

	/**
	 * @brief ������ϵתΪ����ϵ
	 * @return ��������ϵת����ϵ����
	*/
	static glm::mat4 GetTransformMatrixFromRH();

}


