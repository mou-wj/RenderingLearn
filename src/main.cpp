#include "Examples/Artifacts/HairArtifactExample.h"


#include "Framework/Utils/powitacq_rgb.h"
/*


vulkan的framge coordinate
  (0,0)				(width,0)
  +-----------------+
  |		screen   	|
  |					|
  +-----------------+
  (0,height)	    (width,height)

vulkan的采样 coordinate
  (0,0)				(1,0)
  +-----------------+
  |		screen   	|
  |					|
  +-----------------+
  (0,1)	           (1,1)

vulkan的NDC，右手系
向右为x正向，向下为y正向，向内为z正向
+--------------------------------------------+
|	screen					z 内			 |
|						 +                   |
|				 	   +					 |
|				 	 +						 |
|				   +						 |
|				 + + + + + + x	右			 |
|				 +							 |
|				 +							 |
|				 +							 |
|				 +							 |
|				 y	下						 |
+--------------------------------------------+

 */
void TransformTest();
void AutoTest();
extern int ImGuiOpenGLTest();
extern int ImGuiVKTest();
int main()
{

	//AutoTest();

	HairArtifactExample example;
	ExampleBase::Run(&example);


	return 0;
}

void AutoTest() {
	HairArtifactExample example;
	ExampleBase::Run(&example);
}


void TransformTest() {
	auto p = glm::vec4(1, 1, 1, 1);
	auto view = Transform::GetViewMatrix(glm::vec3(0, 0, 3), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
	ShowMat(view);
	ShowVec(view * p);
	auto proj = Transform::GetPerspectiveProj(0.1, 100, 90, 1);
	ShowMat(proj);
	ShowVec(proj * view * p);

}
