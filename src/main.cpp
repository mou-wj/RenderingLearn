#include "Examples/SimpleExamples/DrawSimpleTriangleSample.h"
#include "Examples/SimpleExamples/UniformExample.h"
int main()
{
	//extern void GLSL2SPIRV();
	//GLSL2SPIRV();

	//DrawSimpleTriangleSample exsample;
	UniformExample exsample;
	ExampleBase::Run(&exsample);


	return 0;
}