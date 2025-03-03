#include "Examples/SimpleExamples/DrawSimpleTriangleSample.h"
#include "Examples/SimpleExamples/UniformExample.h"
#include "Examples/SimpleExamples/SkyBoxExample.h"
#include "Examples/SimpleExamples/SimpleSceenExample.h"
#include "Examples/SimpleExamples/TranslucentBlendExample.h"
#include "Examples/SimpleExamples/SimpleTessellationExample.h"
#include "Examples/SimpleExamples/SimpleMeshShaderExample.h"
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
#include "Examples/RealTimeRendering4/C15NonPhotorealisticRenderingExample.h"
#include "Examples/RealTimeRendering4/C17CurvesAndCurvedSurfacesExample.h"
#include "Examples/RealTimeRendering4/C19AccelerationAlgorithmsExample.h"
#include "Examples/RealTimeRendering4/C20EfficientShadingExample.h"
#include "Examples/RealTimeRendering4/C22IntersectionTestMethodsExample.h"
#include "Examples/RealTimeRendering4/C26RealTimeRayTracingExample.h"
#include "Examples/Artifacts/PBRArtifactExample.h"



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
int Vulkankk();

int main()
{
	//Vulkankk();

	//extern void GLSL2SPIRV();
	//GLSL2SPIRV();
	//CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	//CaptureNum(4);
	//AutoTest();
	//TransformTest();
	//DrawSimpleTriangleSample example;
	//UniformExample example;
	//SkyBoxExample example;
	//TranslucentBlendExample example;
	//SimpleSceenExample example;
	//SimpleTessellationExample example;
	//SimpleMeshShaderExample example;
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
	//C14VolumetricandTranslucencyRenderingExample example;
	//C15NonPhotorealisticRenderingExample example;
	//C17CurvesAndCurvedSurfacesExample example;
	//C19AccelerationAlgorithmsExample example;
	//C20EfficientShadingExample example;
	//C22IntersectionTestMethodsExample example;
	//C26RealTimeRayTracingExample example;
	PBRArtifactExample example;
	ExampleBase::Run(&example);


	return 0;
}

void AutoTest() {
	{
		DrawSimpleTriangleSample example;
		ExampleBase::Run(&example);
	}
	{
		UniformExample example;
		ExampleBase::Run(&example);
	}
	{
		SkyBoxExample example;
		ExampleBase::Run(&example);
	}
	{
		TranslucentBlendExample example;
		ExampleBase::Run(&example);
	}
	{
		SimpleSceenExample example;
		ExampleBase::Run(&example);
	}
	{
		SimpleTessellationExample example;
		ExampleBase::Run(&example);
	}
	{
		SimpleMeshShaderExample example;
		ExampleBase::Run(&example);
	}
	{
		SamplePointsExample example;
		ExampleBase::Run(&example);
	}
	{
		ReflectionModelsExample example;
		ExampleBase::Run(&example);
	}
	{
		DisturbutionLobeExample example;
		ExampleBase::Run(&example);
	}
	{
		GeometryShaderExample example;
		ExampleBase::Run(&example);
	}
	{
		VolumeScatteringExample example;
		ExampleBase::Run(&example);
	}
	{
		C5ShadingBasicsExample example;
		ExampleBase::Run(&example);
	}
	{
		C6TexturingExample example;
		ExampleBase::Run(&example);
	}
	{
		C7ShadowsExample example;
		ExampleBase::Run(&example);
	}
	{
		C9PhysicalBasedRenderingExample example;
		ExampleBase::Run(&example);
	}
	{
		C10LocalIlluminationExample example;
		ExampleBase::Run(&example);
	}
	{
		C11GlobalIlluminationExample example;
		ExampleBase::Run(&example);
	}
	{
		C12ImageSpaceEffectesExample example;
		ExampleBase::Run(&example);
	}
	{
		C14VolumetricandTranslucencyRenderingExample example;
		ExampleBase::Run(&example);
	}
	{
		C15NonPhotorealisticRenderingExample example;
		ExampleBase::Run(&example);
	}
	{
		C17CurvesAndCurvedSurfacesExample example;
		ExampleBase::Run(&example);
	}

	{
		C19AccelerationAlgorithmsExample example;
		ExampleBase::Run(&example);
	}
	{
		C20EfficientShadingExample example;
		ExampleBase::Run(&example);
	}
	{
		C22IntersectionTestMethodsExample example;
		ExampleBase::Run(&example);
	}
	{
		C26RealTimeRayTracingExample example;
		ExampleBase::Run(&example);
	}
	{
		PBRArtifactExample example;
		ExampleBase::Run(&example);
	}
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
