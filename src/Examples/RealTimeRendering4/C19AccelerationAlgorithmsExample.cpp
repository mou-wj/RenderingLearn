#include "C19AccelerationAlgorithmsExample.h"



void DeleteTree(TreeNode* treeRoot)
{
	if (!treeRoot)
	{
		return;

	}
	else {
		DeleteTree(treeRoot->left);
		DeleteTree(treeRoot->right);

		delete treeRoot;
	}
}

void C19AccelerationAlgorithmsExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C19AccelerationAlgorithmsExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C19AccelerationAlgorithmsExample.frag";

	ShaderCodePaths shaderMeshCodePath;
	shaderMeshCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C19AccelerationAlgorithmsExample.vert";
	shaderMeshCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C19AccelerationAlgorithmsExampleMesh.frag";


	ShaderCodePaths lodBlendCodePath;
	lodBlendCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C19AccelerationAlgorithmsExample.vert";
	lodBlendCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C19AccelerationAlgorithmsExampleLODBlend.frag";


	renderPassInfos.resize(1);
	renderPassInfos[0].renderTargets.colorAttachment.clearValue = VkClearValue{0.3,0.3,0,1};
	renderPassInfos[0].InitDefaultRenderPassInfo({ shaderMeshCodePath, shaderCodePath,lodBlendCodePath }, windowWidth, windowHeight);
	renderPassInfos[0].renderTargets.enaleInputAttachment = true;
					
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//д����ȸ���
			
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//д����ȸ���

	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassDescription.inputAttachmentCount = 1;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassDescription.pInputAttachments = &renderPassInfos[0].renderTargets.inputAttachmentRef;				
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_FALSE;//д����ȸ���



}

void C19AccelerationAlgorithmsExample::InitResourceInfos()
{


	geoms.resize(3);
	geoms[0].useIndexBuffers = false;
	geoms[1].useIndexBuffers = false;
	geoms[2].useIndexBuffers = false;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/metallic_barrel_with_lod0.obj",geoms[0]);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/metallic_barrel_with_lod1.obj", geoms[1]);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/metallic_barrel_with_lod2.obj", geoms[2]);
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 2 };

	TextureDataSource dataSource;
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/metallic_barrel_with_lod/Scene_-_Root_baseColor.jpg";
	textureBindInfos["baseColor"].textureDataSources.push_back(dataSource);
	textureBindInfos["baseColor"].binding = 0;
	textureBindInfos["baseColor"].pipeId = 1;

	bufferBindInfos["Transform"].size = sizeof(glm::mat4);
	bufferBindInfos["Transform"].binding = 1;

	bufferBindInfos["blendAlpha"].size = sizeof(float);
	bufferBindInfos["blendAlpha"].binding = 2;
	bufferBindInfos["blendAlpha"].pipeId = 2;


}

void C19AccelerationAlgorithmsExample::Loop()
{
	//����BSP tree
	//TreeNode* root = new TreeNode();
	//BuildBSPTree(root, geoms[0].AABBs);

	//DeleteTree(root);
	
	Camera camera(glm::vec3(0, -1, -1), glm::vec3(0, -1, 3), glm::vec3(0, 1, 0));

	glm::mat4 mvpTransform = camera.GetProj() * camera.GetView();

	ShowMatColMajor(mvpTransform);


	FillBuffer(buffers["Transform"], 0, sizeof(glm::mat4), (const char*)&mvpTransform);

	BindBuffer("Transform");

	bufferBindInfos["Transform"].pipeId = 1;
	BindBuffer("Transform");

	bufferBindInfos["Transform"].pipeId = 2;
	BindBuffer("Transform");
	
	BindBuffer("blendAlpha");

	BindTexture("baseColor");
	textureBindInfos["baseColor"].pipeId = 2;
	BindTexture("baseColor");

	//��camera��Ӧ�����Ļص�����
	WindowEventHandler::SetEventCallBack(KEY_W_PRESS, [&camera]() {camera.Move(MoveDirection::FORWARD); }, "���w ���ǰ��");
	WindowEventHandler::SetEventCallBack(KEY_S_PRESS, [&camera]() {camera.Move(MoveDirection::BACK); }, "���s �������");
	WindowEventHandler::SetEventCallBack(KEY_A_PRESS, [&camera]() {camera.Move(MoveDirection::LEFT); }, "���a �������");
	WindowEventHandler::SetEventCallBack(KEY_D_PRESS, [&camera]() {camera.Move(MoveDirection::RIGHT); }, "���d �������");
	WindowEventHandler::SetEventCallBack(KEY_UP_PRESS, [&camera]() {
		//���Ͽ��൱�����е�������ת����z->y,��AROUND_X_NEGATIVE
		camera.Rotate(RotateAction::AROUND_X_NEGATIVE); }, "���up ������Ͽ�");
	WindowEventHandler::SetEventCallBack(KEY_DOWN_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_X_POSITIVE); }, "���down ������¿�");
	WindowEventHandler::SetEventCallBack(KEY_RIGHT_PRESS, [&camera]() {
		//���ҿ��൱�����е�������ת����x->z����AROUND_Y_POSITIVE
		camera.Rotate(RotateAction::AROUND_Y_POSITIVE);
		}, "���right ������ҿ�");
	WindowEventHandler::SetEventCallBack(KEY_LEFT_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_Y_NEGATIVE); }, "���left �������");



	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	//std::vector<char> testImageData;

	//testImageData.resize(512 * 512 * 4);
	//for (uint32_t i = 0; i < 512; i++)
	//{
	//	for (uint32_t j = 0; j < 512; j++)
	//	{
	//		testImageData[(512 * i + j) * 4] = 255;
	//		testImageData[(512 * i + j) * 4 + 1] = 0;
	//		testImageData[(512 * i + j) * 4 + 2] = 0;
	//		testImageData[(512 * i + j) * 4 + 3] = 255;
	//	}


	//}

	//Texture testTexture = Create2DTexture(512, 512, testImageData.data());
	//��¼command buffer

	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	uint32_t curGeoIndex = 0;
	float curBlendAlpha = 0;
	
	CaptureNum(3)
	renderPassInfos[0].truncatedNextSubpassIndex = 2;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		mvpTransform = camera.GetProj() * camera.GetView();
		FillBuffer(buffers["Transform"], 0, sizeof(glm::mat4), (const char*)&mvpTransform);

		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		CmdListWaitFinish(graphicCommandList);
		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);

		//�������Lod�л�����
		//����ģ�͵����Ķ�һ��������ʹ��0��λ���������������,�Ҽ���ģ��û�н�����������任������ģ�����ľ���������ռ��е�����
		float d = glm::distance(camera.GetPos(), geoms[0].AABBcenter);
		//d = 4.7;
		if (d <= 4)
		{

			renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
			renderPassInfos[0].subpassDrawGeoInfos[1] = { 0 };
			curGeoIndex = 0;
			renderPassInfos[0].truncateNextSubpassDraw = true;//��������һ��subpass
		}
		else if (d > 4 && d <= 4.3)
		{
			//����lod�л�
			if (curGeoIndex == 0)
			{
				renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
				renderPassInfos[0].subpassDrawGeoInfos[1] = { 0 };
				renderPassInfos[0].subpassDrawGeoInfos[2] = { 1 };
				curBlendAlpha = (d - 4) / 0.3;
			}
			else {
				renderPassInfos[0].subpassDrawGeoInfos[0] = { 1 };
				renderPassInfos[0].subpassDrawGeoInfos[1] = { 1 };
				renderPassInfos[0].subpassDrawGeoInfos[2] = { 0 };
				curBlendAlpha = (4.3 - d) / 0.3;
			}
			FillBuffer(buffers["blendAlpha"], 0, sizeof(float), (const char*)&curBlendAlpha);
			renderPassInfos[0].truncateNextSubpassDraw = false;//������һ��subpass

		}
		else if (d > 4.3 && d <= 8) {
			curGeoIndex = 1;
			renderPassInfos[0].truncateNextSubpassDraw = true;//��������һ��subpass
			renderPassInfos[0].subpassDrawGeoInfos[0] = { 1 };
			renderPassInfos[0].subpassDrawGeoInfos[1] = { 1 };
			
		}
		else if (d > 8 && d <= 8.3)
		{
			if(curGeoIndex == 1)
			{
				renderPassInfos[0].subpassDrawGeoInfos[0] = { 1 };
				renderPassInfos[0].subpassDrawGeoInfos[1] = { 1 };
				renderPassInfos[0].subpassDrawGeoInfos[2] = { 2 };
				curBlendAlpha = (d - 8) / 0.3;
			}
			else {
				renderPassInfos[0].subpassDrawGeoInfos[0] = { 2 };
				renderPassInfos[0].subpassDrawGeoInfos[1] = { 2 };
				renderPassInfos[0].subpassDrawGeoInfos[2] = { 1 };
				curBlendAlpha = (8.3 - d) / 0.3;
			}
			renderPassInfos[0].truncateNextSubpassDraw = false;//������һ��subpass

			FillBuffer(buffers["blendAlpha"], 0, sizeof(float), (const char*)&curBlendAlpha);

		}
		else if (d > 8.3)
		{
			curGeoIndex = 2;
			renderPassInfos[0].subpassDrawGeoInfos[0] = { 2 };
			renderPassInfos[0].subpassDrawGeoInfos[1] = { 2 };
			renderPassInfos[0].truncateNextSubpassDraw = true;//��������һ��subpass
		}

		//renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };

		//curPassId = 0;

		auto& renderTargets = renderPassInfos[0].renderTargets;


		CmdOpsDrawGeom(graphicCommandList);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachment.attachmentImage,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE,VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachment.attachmentImage, VK_ACCESS_TRANSFER_READ_BIT,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		Present(nexIndex, { finishCopyTargetToSwapchain });
		
		//CopyImageToImage(testTexture.image, swapchainImages[nexIndex], { swapchainImageValidSemaphore }, { presentValidSemaphore });
		//CopyImageToImage(renderTargets.colorAttachment.attachmentImage,testTexture.image, { drawSemaphore }, { presentValidSemaphore });
		//WaitIdle();
		//auto rgba = (const char*)testTexture.image.hostMapPointer;
		//auto r = rgba[0];
		//auto g = rgba[1];
		//auto b = rgba[2];
		//auto a = rgba[3];



	}

}

void C19AccelerationAlgorithmsExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}


void C19AccelerationAlgorithmsExample::BuildBSPTree(TreeNode* curNode, Geometry::AABB curAABB)
{

	using AABB = Geometry::AABB;

	uint32_t numIntersectAABBs = 0;
	uint32_t numContainedAABBs = 0;

	std::map<uint32_t, std::vector<uint32_t>> curIntersectGeoShapeIndexs;



	//�������Σ����Ƿ��к͵�ǰ�߽���ཻ��shape
	for (uint32_t geoId = 0; geoId < geoms.size(); geoId++)
	{
		for (uint32_t i = 0; i < geoms[0].shadeAABBs.size(); i++)
		{
			if (AABB::IsContained(geoms[0].shadeAABBs[i], curAABB))
			{
				numContainedAABBs++;
				curIntersectGeoShapeIndexs[geoId].push_back(i);

			}else if (AABB::CheckAABBsIntersect(curAABB, geoms[0].shadeAABBs[i])) {
				numIntersectAABBs++;
				curIntersectGeoShapeIndexs[geoId].push_back(i);
			}
		}
	}

	if (numContainedAABBs == 1|| (numContainedAABBs == 0 && numIntersectAABBs <= 2))//����һ��������ཻ���������ֹͣ����
	{
		curNode->geoShapeIndexs = curIntersectGeoShapeIndexs;
		return;
	}

	AABB subAABB1 = curAABB, subAABB2 = curAABB;
	
	float xAxisLen = curAABB.maxX - curAABB.minX;
	float yAxisLen = curAABB.maxY - curAABB.minY;
	float zAxisLen = curAABB.maxZ - curAABB.minZ;

	//��ѡ��ǰAABB�������ж��ֻ���
	if (xAxisLen >= yAxisLen && xAxisLen >= zAxisLen)
	{
		subAABB1.maxX -= 0.5 * xAxisLen;
		subAABB2.minX += 0.5 * xAxisLen;
	}

	if (yAxisLen >= xAxisLen && yAxisLen >= zAxisLen)
	{
		subAABB1.maxY -= 0.5 * yAxisLen;
		subAABB2.minY += 0.5 * yAxisLen;
	}

	if (zAxisLen >= yAxisLen && zAxisLen >= xAxisLen)
	{
		subAABB1.maxZ -= 0.5 * zAxisLen;
		subAABB2.minZ += 0.5 * zAxisLen;
	}

	curNode->left = new TreeNode();
	curNode->right = new TreeNode();

	BuildBSPTree(curNode->left, subAABB1);
	BuildBSPTree(curNode->right, subAABB2);

}

