#pragma once
#include "../../Framework/ExampleBase.h"

class C9PhysicalBasedRenderingExample : public ExampleBase
{
public:
	C9PhysicalBasedRenderingExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



