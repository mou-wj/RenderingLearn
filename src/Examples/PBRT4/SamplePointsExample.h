#pragma once
#include "../../Framework/ExampleBase.h"
class SamplePointsExample : public ExampleBase
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



