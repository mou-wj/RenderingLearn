#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C7ShadowsExample : public ExampleBaseVK
{
public:
	C7ShadowsExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



