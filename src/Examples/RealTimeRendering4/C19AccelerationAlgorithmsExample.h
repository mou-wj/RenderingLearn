#pragma once
#include "../../Framework/ExampleBaseVK.h"

struct TreeNode {
	std::map<uint32_t, std::vector<uint32_t>> geoShapeIndexs;
	TreeNode* left;
	TreeNode* right;

	TreeNode() : left(nullptr), right(nullptr) {}
};

class C19AccelerationAlgorithmsExample : public ExampleBaseVK
{
public:
	C19AccelerationAlgorithmsExample() = default;



protected:
	virtual void InitSubPassInfo() override final;
	virtual void InitResourceInfos() override ;//初始化需要的资源
	virtual void Loop() override ;//渲染循环
	virtual void InitSyncObjectNumInfo() override;

private:


	//构建BSP tree中的 k-d tree
	void BuildBSPTree(TreeNode *curNode, Geometry::AABB curAABB);



};



