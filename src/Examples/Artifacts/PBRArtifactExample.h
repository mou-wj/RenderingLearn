#pragma once
#include "../../Framework/ExampleBase.h"

class PBRArtifactExample : public ExampleBase
{
public:
	PBRArtifactExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



