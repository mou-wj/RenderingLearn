#pragma once
#include "../../Framework/ExampleBase.h"

class C12ImageSpaceEffectesExample : public ExampleBase
{
public:
	C12ImageSpaceEffectesExample() = default;



protected:
	virtual void InitComputeInfo() override final;
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//初始化需要的资源
	virtual void Loop() override;//渲染循环
	virtual void InitSyncObjectNumInfo() override;
private:
	//高斯模糊
	void GaussianFilterExample();
	//泛光效果
	void LightBlurExample();



};



