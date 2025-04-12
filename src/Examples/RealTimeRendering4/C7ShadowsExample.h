#pragma once
#include "../../Framework/ExampleBaseVK.h"

class C7ShadowsExample : public ExampleBaseVK
{
public:
	C7ShadowsExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//初始化需要的资源
	virtual void Loop() override;//渲染循环
	virtual void InitSyncObjectNumInfo() override;


};



