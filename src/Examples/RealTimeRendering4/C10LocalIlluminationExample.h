#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C10LocalIlluminationExample : public ExampleBaseVK
{
public:
	C10LocalIlluminationExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



