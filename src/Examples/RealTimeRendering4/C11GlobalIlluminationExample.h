#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C11GlobalIlluminationExample : public ExampleBaseVK
{
public:
	C11GlobalIlluminationExample() = default;



protected:
	virtual void InitComputeInfo() override final;
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



