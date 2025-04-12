#pragma once
#include "../../Framework/ExampleBaseVK.h"
//一个带天空盒的简单场景
class C5ShadingBasicsExample : public ExampleBaseVK
{
public:
	C5ShadingBasicsExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//初始化需要的资源
	virtual void Loop() override;//渲染循环
	virtual void InitSyncObjectNumInfo() override;


};



