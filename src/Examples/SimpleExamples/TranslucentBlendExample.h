#pragma once
#include "../../Framework/ExampleBaseVK.h"
class TranslucentBlendExample : public ExampleBaseVK
{
public:
	TranslucentBlendExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��

	virtual void InitSyncObjectNumInfo() override;

};



