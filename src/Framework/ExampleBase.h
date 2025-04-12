#pragma once


class ExampleBase {

public:
	ExampleBase() = default;
	virtual ~ExampleBase() = default;
	virtual void InitSubPassInfo() = 0;
	virtual void InitResourceInfos() = 0;//初始化需要的资源

	virtual void InitSyncObjectNumInfo() = 0;
	virtual void InitRaytrcingPipelineInfo() {}
	virtual void InitComputeInfo() {}
	virtual void Init() = 0;
	virtual void Loop() = 0;//渲染循环

	
	static void Run(ExampleBase* example);





};