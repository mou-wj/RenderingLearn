#pragma once
#include "../../Framework/ExampleBaseVK.h"

#include <array>

class C22IntersectionTestMethodsExample : public ExampleBaseVK
{
public:
	C22IntersectionTestMethodsExample() = default;



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

	bool IntersectRayAABB(glm::vec3 rayOrigin, glm::vec3 rayDirection, Geometry::AABB& aabb, float& t/*���������ɹ��򷵻ؽ����Ӧ���߷����t����*/);

	bool IntersectRayTriangle(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::array<glm::vec3,3>& trianglePoints, float& t/*���������ɹ��򷵻ؽ����Ӧ���߷����t����*/);


	bool IntersectTwoTriangle(std::array<glm::vec3, 3>& trianglePoints1, std::array<glm::vec3, 3>& trianglePoints2);

private:
	//ʰȡ�ཻ������
	bool PickTriangleFromGeom(Geometry& srcGeo /*Դ��������*/, glm::vec3 rayOrigin, glm::vec3 rayDirection, Geometry& dstPickTrianglesGeo/*Դ�������ݵ�AABBģ������*/);



};



