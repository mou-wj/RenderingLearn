#pragma once

class ExamplePaseBuilderBase {

public:
	ExamplePaseBuilderBase() = default;
	virtual ~ExamplePaseBuilderBase() = default;
	virtual void InitSubPassInfo() override {};
	virtual void InitResourceInfos() override {};//初始化需要的资源

	virtual void InitSyncObjectNumInfo()override {};
	virtual void InitRaytrcingPipelineInfo() {}
	virtual void InitComputeInfo() {}
	virtual void Init() override {};
	virtual void Loop() override {};//渲染循环

	//绘制imgui接口，需要GUI的实例来派生，默认什么为空，什么都不画
	virtual void DrawImGui() {}
};