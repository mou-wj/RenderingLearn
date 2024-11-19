#include "Transform.h"
#include "GlmShowTool.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>

glm::mat4 Transform::GetRotateMatrix(float degree, glm::vec3 rotateAxis)
{
	return glm::rotate(glm::mat4(1.0), glm::radians(degree), glm::normalize(rotateAxis));
}

glm::mat4 Transform::GetScaleMatrix(glm::vec3 scaleVec)
{

	return glm::scale(glm::mat4(1.0), scaleVec);
}

glm::mat4 Transform::GetTranslateMatrix(glm::vec3 translateVec)
{
	return glm::translate(glm::mat4(1.0), translateVec);
}

glm::mat4 Transform::GetInverseMatrix(glm::mat4 srcMatrix)
{
	return glm::inverse(srcMatrix);
}

glm::mat4 Transform::GetTransposeMatrix(glm::mat4 srcMatrix)
{
	return glm::transpose(srcMatrix);
}

glm::mat4 Transform::GetEularRotateMatrix(float yaw, float pitch, float roll)
{
	//列序
	glm::mat4 result(1.0);
	glm::mat4 rotateY(1.0);//yaw
	glm::mat4 rotateX(1.0);//pitch
	glm::mat4 rotateZ(1.0);//row
	float yawRadians = glm::radians(yaw);
	float pitchRadians = glm::radians(pitch);
	float rollRadians = glm::radians(roll);
	rotateY[0][0] = glm::cos(yawRadians);
	rotateY[0][2] = glm::sin(yawRadians);
	rotateY[2][0] = -glm::sin(yawRadians);
	rotateY[2][2] = glm::cos(yawRadians);

	rotateX[1][1] = glm::cos(pitchRadians);
	rotateX[1][2] = glm::sin(pitchRadians);
	rotateX[2][1] = -glm::sin(pitchRadians);
	rotateX[2][2] = glm::cos(pitchRadians);

	rotateZ[0][0] = glm::cos(rollRadians);
	rotateZ[0][1] = glm::sin(rollRadians);
	rotateZ[1][0] = -glm::sin(rollRadians);
	rotateZ[1][1] = glm::cos(rollRadians);

	//result[0][1] = glm::cos(rollRadians) * glm::cos(yawRadians) - glm::sin(rollRadians) * glm::sin(pitchRadians)* glm::sin(yawRadians);
	//result[0][2] = glm::sin(rollRadians) * glm::cos(yawRadians) + glm::cos(rollRadians) * glm::sin(pitchRadians) * glm::sin(yawRadians);
	//result[0][3] = -glm::cos(pitchRadians) * glm::sin(yawRadians);

	//result[1][1] = -glm::sin(rollRadians) * glm::cos(pitchRadians);
	//result[1][2] = glm::cos(rollRadians) * glm::cos(pitchRadians);
	//result[1][3] = glm::sin(pitchRadians);
	//
	//result[2][1] = glm::cos(rollRadians) * glm::cos(yawRadians) + glm::sin(rollRadians) * glm::sin(pitchRadians) * glm::cos(yawRadians);
	//result[2][2] = glm::sin(rollRadians) * glm::sin(yaw) - glm::cos(rollRadians) * glm::sin(pitchRadians) * glm::sin(yawRadians);
	//result[2][3] = glm::cos(pitchRadians) * glm::cos(yawRadians);
	result = rotateZ * rotateX * rotateY;
	return result;
}

glm::mat4 Transform::GetQuaternionMatrix(glm::vec4 center)
{
	glm::mat4  result(1.0);
	result[0][1] = 1 - 2 * (center.y * center.y + center.z * center.z);
	result[0][2] = 2 * (center.x * center.y + center.w * center.z);
	result[0][3] = 2 * (center.x * center.z - center.w * center.y);

	result[1][1] = 2 * (center.x * center.y - center.w * center.z);
	result[1][2] = 1 - 2 * (center.x * center.x + center.z * center.z);
	result[1][3] = 2 * (center.y * center.z + center.w * center.x);

	result[2][1] = 2 * (center.x * center.z + center.w * center.y);
	result[2][2] = 2 * (center.y * center.z - center.w * center.x);
	result[2][3] = 1 - 2 * (center.x * center.x + center.y * center.y);

	return result;
}

glm::mat4 Transform::GetViewMatrix(glm::vec3 pos, glm::vec3 target, glm::vec3 down)
{
	glm::mat4 view(1.0);
	glm::vec3 z = glm::normalize(target - pos);
	glm::vec3 y = glm::normalize(down);
	glm::vec3 x = glm::normalize(glm::cross(y, z));
	//0列
	view[0][0] = x.x;
	view[0][1] = y.x;
	view[0][2] = z.x;
	//1列
	view[1][0] = x.y;
	view[1][1] = y.y;
	view[1][2] = z.y;
	//2列
	view[2][0] = x.z;
	view[2][1] = y.z;
	view[2][2] = z.z;
	//3列
	view[3][0] = - glm::dot(x ,pos);
	view[3][1] = - glm::dot(y, pos);
	view[3][2] = - glm::dot(z, pos);
	return view;
}



glm::mat4 Transform::GetPerspectiveProj(float near, float far, float viewAngle, float ratioWH)
{
	//列主序
	glm::mat4 press(0.0);
	float halfRadias = glm::radians(viewAngle) / 2;
	float nearPlaneH = near * glm::tan(halfRadias);
	float nearPlaneW = nearPlaneH * ratioWH;
	press[0][0] = 1 ;
	press[1][1] = 1 ;
	press[2][2] = (near + far) / near;
	press[3][2] = -far;
	press[2][3] = 1 / near;
	ShowMat(press);
	ShowVec(press * glm::vec4(2, 1, 1, -1));
	glm::mat4 scale(0.0);
	scale[0][0] = 1 / nearPlaneW;
	scale[1][1] = 1 / nearPlaneH;
	scale[2][2] = 1 / (far - near);
	scale[3][2] = near / (near - far);
	scale[3][3] = 1.0;
	ShowMat(scale * press);
	ShowVec(scale * press * glm::vec4(2, 1, 1, -1));
	return scale * press;
}

