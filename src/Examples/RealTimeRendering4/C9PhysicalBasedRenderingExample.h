#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C9PhysicalBasedRenderingExample : public ExampleBaseVK
{
public:
	C9PhysicalBasedRenderingExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



