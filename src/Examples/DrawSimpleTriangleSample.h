#pragma once
#include "../Framework/ExampleBase.h"
class DrawSimpleTriangleSample : public ExampleBase
{
public:
	DrawSimpleTriangleSample() = default;


protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//��ʼ����Ҫ����Դ
	virtual void Loop() override ;//��Ⱦѭ��



};



