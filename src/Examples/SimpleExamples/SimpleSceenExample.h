#pragma once
#include "../../Framework/ExampleBaseVK.h"
//һ������պеļ򵥳���
class SimpleSceenExample : public ExampleBaseVK
{
public:
	SimpleSceenExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



