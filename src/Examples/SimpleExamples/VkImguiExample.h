#pragma once
#include "../../Framework/ExampleBaseVK.h"
class VkImguiExample : public ExampleBaseVK
{
public:
	VkImguiExample() = default;



protected:

	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override;//��ʼ����Ҫ����Դ
	virtual void Loop() override;//��Ⱦѭ��

	virtual void InitSyncObjectNumInfo() override;
	virtual void DrawImGui() override final;

};



