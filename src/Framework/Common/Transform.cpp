#include "Transform.h"

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
	//ÁÐÐò
	glm::mat4 result(1.0);
	float yawRadians = glm::radians(yaw);
	float pitchRadians = glm::radians(pitch);
	float rollRadians = glm::radians(pitch);


	result[0][1] = glm::cos(rollRadians) * glm::cos(yawRadians) - glm::sin(rollRadians) * glm::sin(pitchRadians)* glm::sin(yawRadians);
	result[0][2] = glm::sin(rollRadians) * glm::cos(yawRadians) + glm::cos(rollRadians) * glm::sin(pitchRadians) * glm::sin(yawRadians);
	result[0][3] = -glm::cos(pitchRadians) * glm::sin(yawRadians);

	result[1][1] = -glm::sin(rollRadians) * glm::cos(pitchRadians);
	result[1][2] = glm::cos(rollRadians) * glm::cos(pitchRadians);
	result[1][3] = glm::sin(pitchRadians);
	
	result[2][1] = glm::cos(rollRadians) * glm::cos(yawRadians) + glm::sin(rollRadians) * glm::sin(pitchRadians) * glm::cos(yawRadians);
	result[2][2] = glm::sin(rollRadians) * glm::sin(yaw) - glm::cos(rollRadians) * glm::sin(pitchRadians) * glm::sin(yawRadians);
	result[2][3] = glm::cos(pitchRadians) * glm::cos(yawRadians);

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

glm::mat4 Transform::GetViewMatrix(glm::vec3 pos, glm::vec3 target, glm::vec3 up)
{
	glm::mat4 view(1.0);
	glm::vec3 z = glm::normalize(pos - target);
	glm::vec3 y = glm::normalize(-up);
	glm::vec3 x = glm::normalize(glm::cross(z, y));
	view[0][0] = x.x;
	view[0][1] = x.y;
	view[0][2] = x.z;
	view[0][0] = y.x;
	view[0][1] = y.y;
	view[0][2] = y.z;
	view[0][0] = z.x;
	view[0][1] = z.y;
	view[0][2] = z.z;
	view[0][3] = - pos.x;
	view[1][3] = - pos.y;
	view[2][3] = - pos.z;
	return view;
}

glm::mat4 Transform::GetTransformMatrixFromRH()
{
	glm::mat4 matrix(1.0);
	matrix[2][2] = -1.0f;

	return matrix;
}



glm::mat4 Transform::GetPerspectiveProj(float near, float far, float viewAngle, float ratioWH)
{
	glm::mat4 proj(1.0);
	near = -near;
	far = -far;
	float halfRadias = glm::radians(viewAngle) / 2;
	float nearPlaneH = near * glm::tan(halfRadias);
	float nearPlaneW = nearPlaneH * ratioWH;
	proj[0][0] = 1 ;
	proj[1][1] = 1 ;
	proj[2][2] = (near + far) / near;
	proj[2][3] = -far;
	proj[3][2] = 1 / near;
	glm::mat4 scale(1.0);
	scale[0][0] = 1 / nearPlaneW;
	scale[1][1] = 1 / nearPlaneH;
	scale[2][2] = 1 / (far - near);
	scale[2][3] = near / (near - far);

	return scale * proj;
}

