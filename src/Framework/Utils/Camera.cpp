#include "Camera.h"
#include "../Common/Transform.h"
#include "glm/glm.hpp"

Camera::Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 down, bool isPerspective):pos(pos),target(target),down(down),perspective(isPerspective)
{
	Calculate();
}

void Camera::Move(MoveDirection direction)
{
	glm::vec3 moveVec = z;
	switch (direction)
	{
	case FORWARD:
		moveVec = z;
		break;
	case BACK:
		moveVec = -z;
		break;
	case RIGHT:
		moveVec = x;
		break;
	case LEFT:
		moveVec = -x;
		break;
	case TOP:
		moveVec = -y;
		break;
	case DOWN:
		moveVec = y;
		break;
	default:
		break;
	}
	pos += moveVec * glm::vec3(0.1);
	target += moveVec * glm::vec3(0.1);
	Calculate();


}

void Camera::Rotate(RotateAction rotateAction,float angle)
{
	float yaw = 0, pitch=0, roll = 0;
	switch (rotateAction)
	{
	case AROUND_X_POSITIVE:
		pitch = 1.0 * angle;
		break;
	case AROUND_Y_POSITIVE:
		yaw = 1.0 * angle;
		break;
	case AROUND_Z_POSITIVE:
		roll = 1.0 * angle;

		break;
	case AROUND_X_NEGATIVE:
		pitch = -1.0 * angle;
		break;
	case AROUND_Y_NEGATIVE:
		yaw = -1.0 * angle;
		break;
	case AROUND_Z_NEGATIVE:
		roll = -1.0 * angle;
		break;
	default:
		break;
	}
	rotate = Transform::GetEularRotateMatrix(yaw, pitch, roll);
	Calculate();
}



void Camera::SetProjectType(bool isPerspective)
{
	this->perspective = isPerspective;
	Calculate();
}

void Camera::SetPerspectiveProjectParams(float near, float far, float viewAngle, float ratioWH)
{
	projParams = glm::vec4(near, far, viewAngle, ratioWH);
	Calculate();
}

void Camera::SetParallelProjectParams(float near, float far, float nearPlaneWidth, float nearPlaneHeight)
{
	projParams = glm::vec4(near, far, nearPlaneWidth, nearPlaneHeight);
	Calculate();
}

void Camera::SetCamera(glm::vec3 pos, glm::vec3 target, glm::vec3 down)
{
	this->pos = pos;
	this->target = target;
	this->down = down;
	Calculate();
}

void Camera::SetCamera2(glm::vec3 pos, glm::vec3 forward, glm::vec3 down)
{
	this->pos = pos;
	this->target = pos + forward;
	this->down = down;
	Calculate();
}

void Camera::GenerateRay(glm::vec2 ScreenCoord, glm::vec3& rayOrigin, glm::vec3& rayDirection)
{
	if (perspective)
	{
		rayOrigin = pos;
		float nearPlaneHalfHeight = glm::tan(glm::radians(projParams.z / 2)) * projParams.x;
		float nearPlaneHalfWidth = projParams.w * nearPlaneHalfHeight;

		glm::vec2 nearPlaneSize = glm::vec2(nearPlaneHalfWidth,nearPlaneHalfHeight);


		rayDirection = glm::vec3(nearPlaneSize * ScreenCoord, projParams.x);
		rayDirection = glm::normalize(rayDirection);
	}
	else {
		glm::mat4 viewInverse = glm::inverse(view);
		glm::vec2 halfViewPortWH = glm::vec2(glm::abs(projParams.z) / 2, glm::abs(projParams.w) / 2);
		glm::vec4 o = glm::vec4(ScreenCoord * halfViewPortWH, projParams.x,1);
		o = viewInverse * o;
		o /= o.w;
		rayOrigin = glm::vec3(o);

		glm::vec4 t = glm::vec4(ScreenCoord * halfViewPortWH, projParams.y ,1);
		t = viewInverse * t;
		t /= t.w;
		rayDirection = glm::normalize(glm::vec3(t-o));
	}



}

void Camera::Calculate()
{

	view = Transform::GetViewMatrix(pos, target, down);
	view = rotate * view;
	if (perspective)
	{
		proj = Transform::GetPerspectiveProj(projParams.x, projParams.y, projParams.z, projParams.w);
	}
	else {
		proj = Transform::GetParallelProj(projParams.x, projParams.y, projParams.z, projParams.w);
	}


	x = glm::vec3(view[0][0], view[1][0], view[2][0]);
	y = glm::vec3(view[0][1], view[1][1], view[2][1]);
	z = glm::vec3(view[0][2], view[1][2], view[2][2]);

	down = y;

	//旋转之后target改变
	float distance = glm::distance(pos, target);
	target = pos + z * distance;

	//重置rotate矩阵
	rotate = glm::mat4(1.0);
}
