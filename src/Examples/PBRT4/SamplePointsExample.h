#pragma once
#include "../../Framework/ExampleBaseVK.h"
class SamplePointsExample : public ExampleBaseVK
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



