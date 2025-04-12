#pragma once
#include "../../Framework/ExampleBaseVK.h"

#include <array>

class C22IntersectionTestMethodsExample : public ExampleBaseVK
{
public:
	C22IntersectionTestMethodsExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//初始化需要的资源
	virtual void Loop() override ;//渲染循环
	virtual void InitSyncObjectNumInfo() override;

private:
	void BuildAABBGeometry(Geometry& srcGeo /*源几何数据*/, Geometry& aabbGeo/*源几何数据的AABB模型数据*/);
	void BuildSphereBVGeometry(Geometry& srcGeo /*源几何数据*/, Geometry& sphereBVGeo/*源几何数据的球形包围盒模型数据*/);

private:
	//相交测试
	bool IntersectRaySphere(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 sphereCenter, float radius, float& t/*输出，如果成功则返回交点对应射线方差的t参数*/);

	bool IntersectRayAABB(glm::vec3 rayOrigin, glm::vec3 rayDirection, Geometry::AABB& aabb, float& t/*输出，如果成功则返回交点对应射线方差的t参数*/);

	bool IntersectRayTriangle(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::array<glm::vec3,3>& trianglePoints, float& t/*输出，如果成功则返回交点对应射线方差的t参数*/);


	bool IntersectTwoTriangle(std::array<glm::vec3, 3>& trianglePoints1, std::array<glm::vec3, 3>& trianglePoints2);

private:
	//拾取相交三角形
	bool PickTriangleFromGeom(Geometry& srcGeo /*源几何数据*/, glm::vec3 rayOrigin, glm::vec3 rayDirection, Geometry& dstPickTrianglesGeo/*源几何数据的AABB模型数据*/);



};



