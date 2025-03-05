#pragma once
#include "../../Framework/ExampleBase.h"

class SceenArtifactExample : public ExampleBase
{
public:
	SceenArtifactExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//初始化需要的资源
	virtual void Loop() override;//渲染循环
	virtual void InitSyncObjectNumInfo() override;
private:
	void GenerateLightDepthMap(glm::vec3 lightDir);



};



