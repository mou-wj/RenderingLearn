#pragma once
#include "../Framework/ExampleBase.h"
class DrawSimpleTriangleSample : public ExampleBase
{
public:
	DrawSimpleTriangleSample() = default;


protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//初始化需要的资源
	virtual void Loop() override ;//渲染循环



};



