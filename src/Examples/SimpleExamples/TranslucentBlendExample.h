#pragma once
#include "../../Framework/ExampleBase.h"
class TranslucentBlendExample : public ExampleBase
{
public:
	TranslucentBlendExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��

	virtual void InitSyncObjectNumInfo() override;

};



