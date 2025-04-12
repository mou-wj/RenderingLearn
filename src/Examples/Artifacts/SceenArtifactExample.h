#pragma once
#include "../../Framework/ExampleBaseVK.h"

class SceenArtifactExample : public ExampleBaseVK
{
public:
	SceenArtifactExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;
private:
	void GenerateLightDepthMap(glm::vec3 lightDir);



};



