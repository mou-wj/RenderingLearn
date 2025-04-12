#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C12ImageSpaceEffectesExample : public ExampleBaseVK
{
public:
	C12ImageSpaceEffectesExample() = default;



protected:
	virtual void InitComputeInfo() override final;
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;
private:
	//��˹ģ��
	void GaussianFilterExample();
	//����Ч��
	void LightBlurExample();



};



