#pragma once
#include "../../Framework/ExampleBase.h"
class SamplePointsExample : public ExampleBase
{
public:
	SamplePointsExample() = default;



protected:
	virtual void InitComputeInfo() override final;
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��

	virtual void InitSyncObjectNumInfo() override;

};



