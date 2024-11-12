#include "Examples/SimpleExamples/DrawSimpleTriangleSample.h"
#include "Examples/SimpleExamples/UniformExample.h"

/*


vulkan��framge coordinate
  (0,0)				(width,0)
  +-----------------+
  |					|
  |					|
  +-----------------+
  (0,height)	    (width,height)


vulkan��NDC������ϵ
����Ϊx��������Ϊy��������Ϊz����



 */


int main()
{
	//extern void GLSL2SPIRV();
	//GLSL2SPIRV();
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	//DrawSimpleTriangleSample exsample;
	UniformExample exsample;
	ExampleBase::Run(&exsample);


	return 0;
}