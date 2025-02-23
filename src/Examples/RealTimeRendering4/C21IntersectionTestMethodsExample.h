#pragma once
#include "../../Framework/ExampleBase.h"
class C21IntersectionTestMethodsExample : public ExampleBase
{
public:
	C21IntersectionTestMethodsExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//��ʼ����Ҫ����Դ
	virtual void Loop() override ;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;

private:
	void BuildAABBGeometry(Geometry& srcGeo /*Դ��������*/, Geometry& aabbGeo/*Դ�������ݵ�AABBģ������*/);
	void BuildSphereBVGeometry(Geometry& srcGeo /*Դ��������*/, Geometry& sphereBVGeo/*Դ�������ݵ����ΰ�Χ��ģ������*/);

private:
	//�ཻ����
	bool IntersectRaySphere(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 sphereCenter, float radius, float& t/*���������ɹ��򷵻ؽ����Ӧ���߷����t����*/);


};



