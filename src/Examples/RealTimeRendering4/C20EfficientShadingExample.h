#pragma once
#include "../../Framework/ExampleBase.h"

class C20EfficientShadingExample : public ExampleBase
{
public:
	C20EfficientShadingExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//初始化需要的资源
	virtual void Loop() override;//渲染循环
	virtual void InitSyncObjectNumInfo() override;


};



