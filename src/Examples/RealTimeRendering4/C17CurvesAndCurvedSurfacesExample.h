#pragma once
#include "../../Framework/ExampleBaseVK.h"
class C17CurvesAndCurvedSurfacesExample : public ExampleBaseVK
{
public:
	C17CurvesAndCurvedSurfacesExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//初始化需要的资源
	virtual void Loop() override ;//渲染循环
	virtual void InitSyncObjectNumInfo() override;


};



