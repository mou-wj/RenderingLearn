#pragma once
#include "../../Framework/ExampleBase.h"
class C26RealTimeRayTracingExample : public ExampleBase
{
public:
	C26RealTimeRayTracingExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//初始化需要的资源
	virtual void Loop() override ;//渲染循环
	virtual void InitSyncObjectNumInfo() override;


};



