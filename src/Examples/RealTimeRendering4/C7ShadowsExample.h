#pragma once
#include "../../Framework/ExampleBase.h"

class C7ShadowsExample : public ExampleBase
{
public:
	C7ShadowsExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



