#pragma once


#include <glm/matrix.hpp>

//以下变换在左手系中

namespace Transform {

	/**
 * @brief 获得旋转矩阵，旋转参照右手系，即大拇指指向旋转轴方向，往四指弯曲的方向旋转
 * @param degree 角度（角度值，非弧度）
 * @param rotateAxis 旋转中心轴向量，旋转操作围绕的中心
 * @return 返回旋转矩阵
*/
	glm::mat4 GetRotateMatrix(float degree, glm::vec3 rotateAxis);

	/**
	 * @brief 获得缩放矩阵
	 * @param scaleVec 缩放向量，表示x，y，z方向上缩放的大小
	 * @return 返回缩放向量
	*/
	glm::mat4 GetScaleMatrix(glm::vec3 scaleVec);

	/**
	 * @brief 获得位移矩阵
	 * @param translateVec 位移向量，表示沿着x，y，z方向上位移大小的向量
	 * @return 返回位移矩阵
	*/
	glm::mat4 GetTranslateMatrix(glm::vec3 translateVec);

	/**
	 * @brief 获得逆矩阵
	 * @param srcMatrix 源矩阵
	 * @return 返回源矩阵的逆矩阵
	*/
	glm::mat4 GetInverseMatrix(glm::mat4 srcMatrix);

	/**
	 * @brief 获得转置矩阵
	 * @param srcMatrix 源矩阵
	 * @return 返回源矩阵的转置矩阵
	*/
	glm::mat4 GetTransposeMatrix(glm::mat4 srcMatrix);

	/**
	 * @brief 获得欧拉旋转矩阵，初始看向-z，右手系
	 * @param yaw 偏航角（角度制）类比左右看,绕y轴旋转，x-z,为正表示vulkan的NDC中往左看
	 * @param pitch 俯仰角 （角度制）类比上下看，绕x轴旋转,y-x,为正表示vulkan的NDC中往上看
	 * @param roll 翻滚角 （角度制）类比顺时针看，绕z轴旋转,x-y，为正表示vulkan的NDC中顺时针往下转,等价于图像往逆时针转
	 * @return 返回欧拉旋转矩阵
	*/
	glm::mat4 GetEularRotateMatrix(float yaw, float pitch, float roll);

	/**
	 * @brief 获得四元数旋转矩阵 四元数 q 可以看成是 q = （ sin(f)* u, cos(f) ） = sin(f) * u + cos(f) = exp(f * u)，其中f为三维空间的的一个单位向量，f为一个角度，
	 *		  具体描述见Real-Time Renderering 4th page-80  四元数矩阵表示在空间中 大拇指指向u，沿着四指弯曲的方向旋转2f的角度 右手系
	 * @param center 旋转轴的四元数，（vec3(u) * sin(f),cos(f)）
	 * @return 返回四元数旋转矩阵
	*/
	glm::mat4 GetQuaternionMatrix(glm::vec4 center);

	/**
	 * @brief 不区分坐标系，获取view矩阵，其中down为相机坐标的y，对应vulkan的NDC的向下，target-pos对应相机坐标的z方向，对应vulkan的NDC向内方向
	 * @param pos 相机位置，target 相机视点，down向下向量
	 * @return 返回view矩阵
	*/
	glm::mat4 GetViewMatrix(glm::vec3 pos,glm::vec3 target, glm::vec3 down);



	/**
	 * @brief 获取投影矩阵，不区分坐标系，看向z轴正方向
	 * @param near 近平面，far 远平面，viewAngle，视锥上下之间的夹角，角度值（0-180），ratioWH，视锥近平面的宽高比,near和far必须都大于0，且far大于near
	 * @return 返回透视投影矩阵,视锥体中所有x，y坐标变换到-1到1之间，近平面为0，远平面为1
	*/
	glm::mat4 GetPerspectiveProj(float near, float far, float viewAngle, float ratioWH);


	/**
	 * @brief 获取平行投影矩阵，不区分坐标系，看向z轴正方向
	 * @param near 近平面，far 远平面，width，近平面的宽度，height，近平面的高度
	 * @return 返回平行投影矩阵,视锥体中所有x，y坐标变换到-1到1之间，近平面为0，远平面为1
	*/
	glm::mat4 GetParallelProj(float near, float far, float width, float height);

}


