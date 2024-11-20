#include "Examples/SimpleExamples/DrawSimpleTriangleSample.h"
#include "Examples/SimpleExamples/UniformExample.h"
#include "Examples/SimpleExamples/SkyBoxExample.h"
#include "Examples/SimpleExamples/SimpleSceenExample.h"

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


int main()
{
	//extern void GLSL2SPIRV();
	//GLSL2SPIRV();
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(4);
	//DrawSimpleTriangleSample example;
	//UniformExample example;
	glm::vec4 target=  glm::vec4(0, 0, 1, 1);
	ShowMat(Transform::GetEularRotateMatrix(45, 0, 0));
	ShowVec(Transform::GetEularRotateMatrix(45, 0, 0) * target);
	ShowVec(Transform::GetEularRotateMatrix(45, 0, 0) * glm::vec4(1,0,1,1));
	SkyBoxExample example;
	//SimpleSceenExample example;
	ExampleBase::Run(&example);


	return 0;
}