#include "Examples/SimpleExamples/DrawSimpleTriangleSample.h"
#include "Examples/SimpleExamples/UniformExample.h"
#include "Examples/SimpleExamples/SkyBoxExample.h"

/*


vulkan的framge coordinate
  (0,0)				(width,0)
  +-----------------+
  |					|
  |					|
  +-----------------+
  (0,height)	    (width,height)


vulkan的NDC，左手系
向右为x正向，向下为y正向，向外为z正向



 */


int main()
{
	//extern void GLSL2SPIRV();
	//GLSL2SPIRV();
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	DrawSimpleTriangleSample exsample;
	//SkyBoxExample exsample;
	ExampleBase::Run(&exsample);


	return 0;
}