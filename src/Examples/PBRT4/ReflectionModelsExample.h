#pragma once
#include "../../Framework/ExampleBaseVK.h"
//һ������պеķ���ģ��ʾ��
class ReflectionModelsExample : public ExampleBaseVK
{
public:
	ReflectionModelsExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



