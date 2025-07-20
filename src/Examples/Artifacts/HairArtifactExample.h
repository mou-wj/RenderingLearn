#pragma once
#include "../../Framework/ExampleBaseVK.h"
class HairArtifactExample : public ExampleBaseVK
{
public:
	HairArtifactExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//初始化需要的资源
	virtual void Loop() override;//渲染循环

	virtual void InitSyncObjectNumInfo() override;
	virtual void DrawImGui() override final;

};



