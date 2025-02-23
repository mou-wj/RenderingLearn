#pragma once
#include <glm/matrix.hpp>
enum MoveDirection {
	FORWARD,
	BACK,
	RIGHT,
	LEFT,
	TOP,
	DOWN
};
enum RotateAction {
	AROUND_X_POSITIVE,
	AROUND_Y_POSITIVE,
	AROUND_Z_POSITIVE,
	AROUND_X_NEGATIVE,
	AROUND_Y_NEGATIVE,
	AROUND_Z_NEGATIVE
};


class Camera {
public:
	Camera(glm::vec3 pos = glm::vec3(0, 0, 0), glm::vec3 target = glm::vec3(0, 0, 1), glm::vec3 down = glm::vec3(0, 1, 0));


	void Move(MoveDirection direction);
	void Rotate(RotateAction rotateAction, float angle = 1.0/*旋转角度*/);

	const glm::mat4& GetView() const { return view; }
	const glm::mat4& GetProj() const { return proj; }
	const glm::vec3& GetPos() const { return pos; }

	void SetProjectType(bool isPerspective);
	void SetPerspectiveProjectParams(float near,float far,float viewAngle,float ratioWH);
	void SetParallelProjectParams(float near, float far, float nearPlaneWidth, float nearPlaneHeight);
	
	void SetCamera(glm::vec3 pos, glm::vec3 target, glm::vec3 down);
	void SetCamera2(glm::vec3 pos, glm::vec3 forward, glm::vec3 down);

	void GenerateRay(glm::vec2 ScreenCoord/*屏幕的纹理坐标，x从左到右，y从上到下，范围[-1,1]*/, glm::vec3& rayOrigin, glm::vec3& rayDirection);

private:
	void Calculate();

private:
	glm::vec3 pos = glm::vec3(0.0), target = glm::vec3(0, 0, 1),down = glm::vec3(0,1,0);

	glm::vec3 x = glm::vec3(1,0,0), y = glm::vec3(0, 1, 0), z = glm::vec3(0, 0, 1);


	glm::mat4 view = glm::mat4(1.0);
	glm::mat4 rotate = glm::mat4(1.0);
	glm::mat4 proj = glm::mat4(1.0);
	bool perspective = true;//默认为透视投影，如果不是则为平行投影

	glm::vec4 projParams = glm::vec4(0.1, 100, 90, 1);
};