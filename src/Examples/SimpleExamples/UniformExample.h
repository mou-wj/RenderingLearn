#pragma once
#include "../../Framework/ExampleBaseVK.h"
class UniformExample : public ExampleBaseVK
{
public:
	UniformExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��

	virtual void InitSyncObjectNumInfo() override;

};



