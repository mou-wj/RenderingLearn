#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C14VolumetricandTranslucencyRenderingExample : public ExampleBaseVK
{
public:
	C14VolumetricandTranslucencyRenderingExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



