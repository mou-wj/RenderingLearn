#pragma once
#include "../../Framework/ExampleBase.h"
class C16CurvesAndCurvedSurfacesExample : public ExampleBase
{
public:
	C16CurvesAndCurvedSurfacesExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//��ʼ����Ҫ����Դ
	virtual void Loop() override ;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;


};



