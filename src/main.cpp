#include "Examples/DrawSimpleTriangleSample.h"
int main()
{
	//extern void GLSL2SPIRV();
	//GLSL2SPIRV();
	extern void RenderDocCapTest();
	RenderDocCapTest();

	DrawSimpleTriangleSample sample;
	sample.Run(&sample);


	return 0;
}