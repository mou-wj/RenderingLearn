#pragma once
#include "../../Framework/ExampleBaseVK.h"

class PBRArtifactExample : public ExampleBaseVK
{
public:
	PBRArtifactExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



