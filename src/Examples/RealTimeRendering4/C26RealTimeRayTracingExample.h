#pragma once
#include "../../Framework/ExampleBase.h"
class C26RealTimeRayTracingExample : public ExampleBase
{
public:
	C26RealTimeRayTracingExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//��ʼ����Ҫ����Դ
	virtual void Loop() override ;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



