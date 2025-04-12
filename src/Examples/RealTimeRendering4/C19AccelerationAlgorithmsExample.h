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
	virtual void InitResourceInfos() override ;//��ʼ����Ҫ����Դ
	virtual void Loop() override ;//��Ⱦѭ��
	virtual void InitSyncObjectNumInfo() override;

private:


	//����BSP tree�е� k-d tree
	void BuildBSPTree(TreeNode *curNode, Geometry::AABB curAABB);



};



