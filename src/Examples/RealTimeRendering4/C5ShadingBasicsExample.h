#pragma once
#include "../../Framework/ExampleBase.h"
//һ������պеļ򵥳���
class C5ShadingBasicsExample : public ExampleBase
{
public:
	C5ShadingBasicsExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



