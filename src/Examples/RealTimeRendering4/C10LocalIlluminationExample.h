#pragma once
#include "../../Framework/ExampleBase.h"

class C10LocalIlluminationExample : public ExampleBase
{
public:
	C10LocalIlluminationExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



