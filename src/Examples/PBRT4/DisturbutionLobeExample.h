#pragma once
#include "../../Framework/ExampleBaseVK.h"
//һ������պеļ򵥳���
class DisturbutionLobeExample : public ExampleBaseVK
{
public:
	DisturbutionLobeExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



