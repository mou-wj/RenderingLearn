#pragma once
#include "../../Framework/ExampleBaseVK.h"
class SkyBoxExample : public ExampleBaseVK
{
public:
	SkyBoxExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



