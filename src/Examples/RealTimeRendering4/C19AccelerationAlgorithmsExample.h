#pragma once
#include "../../Framework/ExampleBase.h"
class C19AccelerationAlgorithmsExample : public ExampleBase
{
public:
	C19AccelerationAlgorithmsExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//��ʼ����Ҫ����Դ
	virtual void Loop() override ;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



