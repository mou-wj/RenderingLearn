#pragma once
#include "../../Framework/ExampleBase.h"

class C11GlobalIlluminationExample : public ExampleBase
{
public:
	C11GlobalIlluminationExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



