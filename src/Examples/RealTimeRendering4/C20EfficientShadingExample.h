#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C20EfficientShadingExample : public ExampleBaseVK
{
public:
	C20EfficientShadingExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



