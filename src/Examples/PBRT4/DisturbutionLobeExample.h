#pragma once
#include "../../Framework/ExampleBase.h"
//һ������պеļ򵥳���
class DisturbutionLobeExample : public ExampleBase
{
public:
	DisturbutionLobeExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



