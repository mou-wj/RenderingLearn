#pragma once
#include "../../Framework/ExampleBaseVK.h"
class C17CurvesAndCurvedSurfacesExample : public ExampleBaseVK
{
public:
	C17CurvesAndCurvedSurfacesExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//��ʼ����Ҫ����Դ
	virtual void Loop() override ;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



