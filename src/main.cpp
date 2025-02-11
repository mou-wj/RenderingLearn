#include "Examples/SimpleExamples/DrawSimpleTriangleSample.h"
#include "Examples/SimpleExamples/UniformExample.h"
#include "Examples/SimpleExamples/SkyBoxExample.h"
#include "Examples/SimpleExamples/SimpleSceenExample.h"
#include "Examples/SimpleExamples/TranslucentBlendExample.h"
#include "Examples/PBRT4/SamplePointsExample.h"
#include "Examples/PBRT4/ReflectionModelsExample.h"
#include "Examples/PBRT4/DisturbutionLobeExample.h"
#include "Examples/SimpleExamples/GeometryShaderExample.h"
#include "Examples/PBRT4/VolumeScatteringExample.h"
#include "Examples/RealTimeRendering4/C5ShadingBasicsExample.h"
#include "Examples/RealTimeRendering4/C6TexturingExample.h"
#include "Examples/RealTimeRendering4/C7ShadowsExample.h"
#include "Examples/RealTimeRendering4/C9PhysicalBasedRenderingExample.h"
#include "Examples/RealTimeRendering4/C10LocalIlluminationExample.h"
#include "Examples/RealTimeRendering4/C11GlobalIlluminationExample.h"
#include "Examples/RealTimeRendering4/C12ImageSpaceEffectesExample.h"
#include "Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExample.h"


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

int main()
{
	//extern void GLSL2SPIRV();
	//GLSL2SPIRV();
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(4);
	//TransformTest();
	//DrawSimpleTriangleSample example;
	//UniformExample example;
	//SkyBoxExample example;
	//TranslucentBlendExample example;
	//SimpleSceenExample example;
	//SamplePointsExample example;
	//powitacq_rgb::BRDF brdf(std::string(PROJECT_DIR) + "/resources/mesured_bsdf/aniso_brushed_aluminium_1_rgb.bsdf");
	

	//ReflectionModelsExample example;
	//DisturbutionLobeExample example;
	//GeometryShaderExample example;
	//VolumeScatteringExample example;
	//C5ShadingBasicsExample example;
	//C6TexturingExample example;
	//C7ShadowsExample example;
	//C9PhysicalBasedRenderingExample example;
	//C10LocalIlluminationExample example;
	//C11GlobalIlluminationExample example;
	//C12ImageSpaceEffectesExample example;
	C14VolumetricandTranslucencyRenderingExample example;
	ExampleBase::Run(&example);


	return 0;
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
