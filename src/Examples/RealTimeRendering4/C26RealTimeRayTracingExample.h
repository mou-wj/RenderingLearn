#pragma once
#include "../../Framework/ExampleBaseVK.h"
class C26RealTimeRayTracingExample : public ExampleBaseVK
{
public:
	C26RealTimeRayTracingExample() = default;



protected:
	virtual void InitRaytrcingPipelineInfo() override final;
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//��ʼ����Ҫ����Դ
	virtual void Loop() override ;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



