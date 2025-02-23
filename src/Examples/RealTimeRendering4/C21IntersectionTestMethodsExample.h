#pragma once
#include "../../Framework/ExampleBase.h"
class C21IntersectionTestMethodsExample : public ExampleBase
{
public:
	C21IntersectionTestMethodsExample() = default;



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


};



