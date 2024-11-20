#include "Camera.h"
#include "../Common/Transform.h"

Camera::Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 down):pos(pos),target(target),down(down)
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

void Camera::Calculate()
{

	view = Transform::GetViewMatrix(pos, target, down);
	view = rotate * view;
	proj = Transform::GetPerspectiveProj(0.1, 100, 90, 1);

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
