#pragma once
#include "../../Framework/ExampleBaseVK.h"
class SimpleTessellationExample : public ExampleBaseVK
{
public:
	SimpleTessellationExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//初始化需要的资源
	virtual void Loop() override ;//渲染循环
	virtual void InitSyncObjectNumInfo() override;


};



