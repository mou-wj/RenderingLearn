#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C15NonPhotorealisticRenderingExample : public ExampleBaseVK
{
public:
	C15NonPhotorealisticRenderingExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



