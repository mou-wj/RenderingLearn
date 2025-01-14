#include "C6TexturingExample.h"
#include "glm/mat4x4.hpp"

void C6TexturingExample::InitSubPassInfo()
{
	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C6TexturingExample.vert";
	drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C6TexturingExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C6TexturingExample.frag";



	//InitDefaultGraphicSubpassInfo();
	renderPassInfos.resize(1);

	auto& subpassInfo = renderPassInfos[0].subpassInfo;
	auto& renderTargets = renderPassInfos[0].renderTargets;

	subpassInfo.subpassDescs.resize(1);
	//������ɫ��·��
	subpassInfo.subpassDescs[0].pipelinesShaderCodePaths = drawSceenCodePath;
	//��ʼ������״̬
	subpassInfo.subpassDescs[0].subpassPipelineStates.Init(windowWidth, windowWidth);

	//�����޳�
	subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;


	auto& subpassDesc1 = subpassInfo.subpassDescs[0];
	subpassDesc1.subpassDescription.flags = 0;
	subpassDesc1.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc1.subpassDescription.inputAttachmentCount = 0;
	subpassDesc1.subpassDescription.pInputAttachments = nullptr;
	subpassDesc1.subpassDescription.colorAttachmentCount = 1;
	subpassDesc1.subpassDescription.pColorAttachments = &renderTargets.colorRef;
	subpassDesc1.subpassDescription.pResolveAttachments = nullptr;
	subpassDesc1.subpassDescription.pDepthStencilAttachment = &renderTargets.depthRef;
	subpassDesc1.subpassDescription.preserveAttachmentCount = 0;
	subpassDesc1.subpassDescription.pPreserveAttachments = nullptr;


	subpassInfo.subpassDepends.resize(1);
	auto& subpassDepend1 = subpassInfo.subpassDepends[0];
	subpassDepend1.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDepend1.dstSubpass = 0;
	subpassDepend1.dependencyFlags = 0;
	subpassDepend1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDepend1.srcAccessMask = 0;
	subpassDepend1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDepend1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


}

void C6TexturingExample::InitResourceInfos()
{

	geoms.resize(1);
	geoms[0].useIndexBuffers = false;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/plane.obj",geoms[0]);

	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };



	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 0;

	bufferBindInfos["Info"].size = sizeof(glm::vec4) * 3;
	bufferBindInfos["Info"].binding = 3;
	bufferBindInfos["Info"].pipeId = 0;
	
	//�ڰ���������
	std::vector<char> bwDatas(512 * 512 * 4);
	uint32_t pattern[2][2] = { 0,1,1,0 };
	uint32_t blockWidth = 32;
	for (uint32_t i = 0; i < 512; i++)
	{
		for (uint32_t j = 0; j < 512; j++)
		{
			uint32_t i_pat = i / blockWidth % 2;
			uint32_t j_pat = j / blockWidth % 2;
			uint32_t curP = pattern[i_pat][j_pat];
			uint8_t r = 0, g = 0, b = 0;
			if (curP == 0)
			{
				r = g = b = 255;
			}
			bwDatas[(512 * i + j) * 4] = r;
			bwDatas[(512 * i + j) * 4 + 1] = g;
			bwDatas[(512 * i + j) * 4 + 2] = b;
			bwDatas[(512 * i + j) * 4 + 3] = 255;

		}


	}


	//����SAT������SATÿ�����ص�ֵ��ŵ��Ǻͣ������ۼӻᵼ�º�Խ��Խ��Ȼ������ֵ���������SAT�в�ȡ��źͳ������ص�����ķ�ʽ
	//SATĿǰ�ƺ����ڴ洢��ʽ�ľ���������������
	std::vector<float> bwSATDatas(512 * 512 * 4);
	for (uint32_t i = 0; i < 512; i++)
	{
		for (uint32_t j = 0; j < 512; j++)
		{
			uint32_t curNumPixel = (i + 1) * (j + 1);
			float areaSumR = 0;
			float areaSumG = 0;
			float areaSumB = 0;
			int preI = i - 1;
			int preJ = j - 1;
			if (preI >= 0)
			{
				areaSumR += bwSATDatas[(preI * 512 + j) * 4];
				areaSumG += bwSATDatas[(preI * 512 + j) * 4 + 1];
				areaSumB += bwSATDatas[(preI * 512 + j) * 4 + 2];

			}
			if (preJ >= 0)
			{
				areaSumR += bwSATDatas[(i * 512 + preJ) * 4];
				areaSumG += bwSATDatas[(i * 512 + preJ) * 4 + 1];
				areaSumB += bwSATDatas[(i * 512 + preJ) * 4 + 2];

			}
			if (preI >= 0 && preJ >= 0)
			{
				areaSumR -= bwSATDatas[(preI * 512 + preJ) * 4];
				areaSumG -= bwSATDatas[(preI * 512 + preJ) * 4 + 1];
				areaSumB -= bwSATDatas[(preI * 512 + preJ) * 4 + 2];
			}

			float cR = (uint8_t)bwDatas[(i * 512 + j) * 4] / 255.0;
			float cG = (uint8_t)bwDatas[(i * 512 + j) * 4 + 1] / 255.0;
			float cB = (uint8_t)bwDatas[(i * 512 + j) * 4 + 2] / 255.0;

			areaSumR += cR;
			areaSumG += cG;
			areaSumB += cB;


			//bwSATDatas[(512 * i + j) * 4] = areaSumR / curNumPixel;
			//bwSATDatas[(512 * i + j) * 4 + 1] = areaSumG / curNumPixel;
			//bwSATDatas[(512 * i + j) * 4 + 2] = areaSumB / curNumPixel;
			bwSATDatas[(512 * i + j) * 4] = areaSumR;
			bwSATDatas[(512 * i + j) * 4 + 1] = areaSumG;
			bwSATDatas[(512 * i + j) * 4 + 2] = areaSumB;
			bwSATDatas[(512 * i + j) * 4 + 3] = 1;

		}


	}


	TextureDataSource dataSource;
	dataSource.height = 512;
	dataSource.width = 512;
	dataSource.imagePixelDatas = bwDatas;
	//dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/black-white.jpg";
	textureBindInfos["bw"].textureDataSources.push_back(dataSource);
	textureBindInfos["bw"].binding = 1;
	textureBindInfos["bw"].buildMipmap = true;

	TextureDataSource satDataSource;
	satDataSource.height = 512;
	satDataSource.width = 512;
	satDataSource.imagePixelDatas.resize(bwSATDatas.size() * sizeof(float));
	std::memcpy(satDataSource.imagePixelDatas.data(), bwSATDatas.data(), satDataSource.imagePixelDatas.size());
	//dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/black-white.jpg";
	textureBindInfos["satbw"].textureDataSources.push_back(satDataSource);
	textureBindInfos["satbw"].binding = 2;
	textureBindInfos["satbw"].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureBindInfos["satbw"].formatComponentByteSize = sizeof(float);
	

	//һ��alpha��ͼ
	TextureDataSource alphaDataSource;
	alphaDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/material/AsphaltDamageSet001_1K-JPG/AsphaltDamageSet001.png";
	textureBindInfos["alphaT"].textureDataSources.push_back(alphaDataSource);
	textureBindInfos["alphaT"].binding = 4;

	//һ��������ͼ
	TextureDataSource normalDataSource;
	normalDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/material/AsphaltDamageSet001_1K-JPG/AsphaltDamageSet001_1K-JPG_NormalGL.jpg";
	textureBindInfos["alphaT"].textureDataSources.push_back(normalDataSource);
	textureBindInfos["alphaT"].binding = 5;
	

	//һ������albedo��ͼ
	TextureDataSource albedoDataSource;
	albedoDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/material/Wood066_1K-JPG/Wood066_1K-JPG_Color.jpg";
	textureBindInfos["albedo"].textureDataSources.push_back(albedoDataSource);
	textureBindInfos["albedo"].binding = 6;
}

void C6TexturingExample::Loop()
{
	uint32_t i = 0;;
	uint32_t exampleType = 0;


	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,-1,-12),glm::vec3(0,0,0),glm::vec3(0,1,0));
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
	WindowEventHandler::SetEventCallBack(KEY_I_PRESS, [&]() {
		std::cout << "����һ������ֵ: ��Χ[0,6]: " << std::endl;
		uint32_t tmp = 0;
		std::cin >> tmp;
		if (tmp > 6)
		{
			std::cout << "����Ƿ���" << std::endl;
			return;
		}
		exampleType = tmp;
		 }, "���i ������ʾ��ʵ����: 0: ֱ�ӻ�ȡ����ֵ ; 1:mipmap ;2: SAT; 3:������ɵ�����;4:������;5:��������;6:alpha��ͼ");
	std::cout << "��ǰΪ0: ֱ�ӻ�ȡ����ֵ " << std::endl;


	struct Buffer {
		glm::mat4 world;
		glm::mat4 view;
		glm::mat4 proj;
	} buffer;
	buffer.world = glm::mat4(1.0);
	buffer.view = camera.GetView();
	buffer.proj = camera.GetProj();




	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };


	//��uniform buffer
	BindBuffer("SimpleSceenExampleBuffer");
	BindBuffer("Info");
	BindTexture("bw");
	BindTexture("satbw");
	BindTexture("alphaT");
	BindTexture("albedo");

	textures["satbw"].image.WriteToJpg("satbw.jpg",0,0);
	float delta = 0.5;


	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;

		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
		delta += 0.001;
		if (delta > 1) {
			delta = 0;
		}
		FillBuffer(buffers["Info"], 0, sizeof(float), (const char*)&delta);
		FillBuffer(buffers["Info"], sizeof(glm::vec4), sizeof(glm::vec3), (const char*)&camera.GetPos());
		FillBuffer(buffers["Info"], sizeof(glm::vec4) * 2, sizeof(uint32_t), (const char*)&exampleType);

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
		WindowEventHandler::ProcessEvent();
		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		
		CmdOpsDrawGeom(graphicCommandList);
		
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachment.attachmentImage, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachment.attachmentImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		Present(nexIndex, { finishCopyTargetToSwapchain });



	}
	WaitIdle();
}


void C6TexturingExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
