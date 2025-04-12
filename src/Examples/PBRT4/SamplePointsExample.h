#pragma once
#include "../../Framework/ExampleBaseVK.h"
class SamplePointsExample : public ExampleBaseVK
{
public:
	SamplePointsExample() = default;



protected:
	virtual void InitComputeInfo() override final;
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//初始化需要的资源
	virtual void Loop() override;//渲染循环

	virtual void InitSyncObjectNumInfo() override;

};



