#pragma once
#include "../../Framework/ExampleBase.h"

class C20EfficientShadingExample : public ExampleBase
{
public:
	C20EfficientShadingExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



