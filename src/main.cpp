#include "Examples/SimpleExamples/DrawSimpleTriangleSample.h"
#include "Examples/SimpleExamples/UniformExample.h"
#include "Examples/SimpleExamples/SkyBoxExample.h"

/*


vulkan��framge coordinate
  (0,0)				(width,0)
  +-----------------+
  |		screen   	|
  |					|
  +-----------------+
  (0,height)	    (width,height)

vulkan�Ĳ��� coordinate
  (0,0)				(1,0)
  +-----------------+
  |		screen   	|
  |					|
  +-----------------+
  (0,1)	           (1,1)

vulkan��NDC������ϵ
����Ϊx��������Ϊy��������Ϊz����
+--------------------------------------------+
|	screen					z ��			 |
|						 +                   |
|				 	   +					 |
|				 	 +						 |
|				   +						 |
|				 + + + + + + x	��			 |
|				 +							 |
|				 +							 |
|				 +							 |
|				 +							 |
|				 y	��						 |
+--------------------------------------------+

 */


int main()
{
	//extern void GLSL2SPIRV();
	//GLSL2SPIRV();
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	DrawSimpleTriangleSample example;
	//UniformExample example;
	//SkyBoxExample example;
	ExampleBase::Run(&example);


	return 0;
}