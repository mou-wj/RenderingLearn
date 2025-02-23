#include "C21IntersectionTestMethodsExample.h"

void C21IntersectionTestMethodsExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C21IntersectionTestMethodsExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C21IntersectionTestMethodsExample.frag";
	
	ShaderCodePaths BVMeshCodePath;
	BVMeshCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C21IntersectionTestMethodsExample.vert";
	BVMeshCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C21IntersectionTestMethodsExampleBVMesh.frag";

	renderPassInfos.resize(1);
	renderPassInfos[0].InitDefaultRenderPassInfo({ shaderCodePath,BVMeshCodePath }, windowWidth, windowHeight);


	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件
	
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件
}

void C21IntersectionTestMethodsExample::InitResourceInfos()
{

	
	geoms.resize(3);

	auto& geom = geoms[0];
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/monkey.obj", geom);
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[0].subpassDrawGeoInfos[1] = { 1,2 };
	//构建AABB geo
	BuildAABBGeometry(geom, geoms[1]);
	BuildSphereBVGeometry(geom, geoms[2]);

	bufferBindInfos["Transform"].size = sizeof(glm::mat4);
	bufferBindInfos["Transform"].binding = 0;

	bufferBindInfos["IntersectInfo"].size = sizeof(uint32_t);
	bufferBindInfos["IntersectInfo"].binding = 1;
	bufferBindInfos["IntersectInfo"].pipeId = 1;
}

void C21IntersectionTestMethodsExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");

	Camera camera(glm::vec3(0, -1, -5), glm::vec3(0, -1, 3), glm::vec3(0, 1, 0));

	//绑定camera响应按键的回调函数
	WindowEventHandler::SetEventCallBack(KEY_W_PRESS, [&camera]() {camera.Move(MoveDirection::FORWARD); }, "点击w 相机前移");
	WindowEventHandler::SetEventCallBack(KEY_S_PRESS, [&camera]() {camera.Move(MoveDirection::BACK); }, "点击s 相机后移");
	WindowEventHandler::SetEventCallBack(KEY_A_PRESS, [&camera]() {camera.Move(MoveDirection::LEFT); }, "点击a 相机左移");
	WindowEventHandler::SetEventCallBack(KEY_D_PRESS, [&camera]() {camera.Move(MoveDirection::RIGHT); }, "点击d 相机右移");
	WindowEventHandler::SetEventCallBack(KEY_UP_PRESS, [&camera]() {
		//往上看相当于所有点往下旋转，即z->y,即AROUND_X_NEGATIVE
		camera.Rotate(RotateAction::AROUND_X_NEGATIVE); }, "点击up 相机往上看");
	WindowEventHandler::SetEventCallBack(KEY_DOWN_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_X_POSITIVE); }, "点击down 相机往下看");
	WindowEventHandler::SetEventCallBack(KEY_RIGHT_PRESS, [&camera]() {
		//往右看相当于所有点往左旋转，即x->z，即AROUND_Y_POSITIVE
		camera.Rotate(RotateAction::AROUND_Y_POSITIVE);
		}, "点击right 相机往右看");
	WindowEventHandler::SetEventCallBack(KEY_LEFT_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_Y_NEGATIVE); }, "点击left 相机往左看");
	WindowEventHandler::SetEventCallBack(MOUSE_LEFT_BUTTON_CLICKED, [&camera,this]() {
		//获取鼠标点击事件的坐标
		auto pos = WindowEventHandler::GetMousePos();
		float halfW = windowWidth / 2, halfH = windowHeight / 2;
		float normalizeX = (pos[0] - halfW)/halfW;
		float normalizeY = (pos[0] - halfH)/halfH;

		glm::vec3 rayOrigin, rayDirection;
		camera.GenerateRay(glm::vec2(normalizeX,normalizeY), rayOrigin, rayDirection);
		float t = 0;
		auto intersect = IntersectRaySphere(rayOrigin, rayDirection, geoms[0].AABBcenter, geoms[0].BVSphereRadius, t);
		FillBuffer(buffers["IntersectInfo"], 0, sizeof(bool), (const char*)&intersect);
		
		ShowVec(rayDirection);
		}, "点击left 相机往左看");


	BindBuffer("Transform");
	bufferBindInfos["Transform"].pipeId = 1;
	BindBuffer("Transform");

	BindBuffer("IntersectInfo");


	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	CaptureNum(3);

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);
		glm::mat4 transform = camera.GetProj() * camera.GetView();//不进行世界坐标变换
		FillBuffer(buffers["Transform"], 0, sizeof(glm::mat4), (const char*)&transform);
		CmdListWaitFinish(graphicCommandList);
		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE,VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, VK_ACCESS_TRANSFER_READ_BIT,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		Present(nexIndex, { finishCopyTargetToSwapchain });

	}

}

void C21IntersectionTestMethodsExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}

void C21IntersectionTestMethodsExample::BuildAABBGeometry(Geometry& srcGeo, Geometry& aabbGeo)
{
	Geometry::AABB& aabb = srcGeo.AABBs;


	struct Vertex {
		float x, y, z;
	};

	// 获取AABB对应的顶点数据
	static std::vector<Vertex> vertices{

		{aabb.minX, aabb.minY, aabb.minZ}, // 0: (minX, minY, minZ)
		{ aabb.minX, aabb.minY, aabb.maxZ }, // 1: (minX, minY, maxZ)
		{ aabb.minX, aabb.maxY, aabb.minZ }, // 2: (minX, maxY, minZ)
		{ aabb.minX, aabb.maxY, aabb.maxZ }, // 3: (minX, maxY, maxZ)
		{ aabb.maxX, aabb.minY, aabb.minZ }, // 4: (maxX, minY, minZ)
		{ aabb.maxX, aabb.minY, aabb.maxZ }, // 5: (maxX, minY, maxZ)
		{ aabb.maxX, aabb.maxY, aabb.minZ }, // 6: (maxX, maxY, minZ)
		{ aabb.maxX, aabb.maxY, aabb.maxZ }  // 7: (maxX, maxY, maxZ)

	};

	// 获取AABB对应的三角形索引数据
	static std::vector<unsigned int> indices{
		// 底面
		0, 2, 4, 4, 2, 6,  // 底面： (minX, minY, minZ) -> (minX, maxY, minZ) -> (maxX, minY, minZ)
		// 顶面
		1, 5, 3, 3, 5, 7,  // 顶面： (minX, minY, maxZ) -> (maxX, minY, maxZ) -> (minX, maxY, maxZ)
		// 前面
		2, 3, 6, 6, 3, 7,  // 前面： (minX, maxY, minZ) -> (minX, maxY, maxZ) -> (maxX, maxY, minZ)
		// 后面
		0, 4, 1, 1, 4, 5,  // 后面： (minX, minY, minZ) -> (maxX, minY, minZ) -> (minX, minY, maxZ)
		// 左面
		0, 1, 2, 2, 1, 3,  // 左面： (minX, minY, minZ) -> (minX, minY, maxZ) -> (minX, maxY, minZ)
		// 右面
		4, 6, 5, 5, 6, 7   // 右面： (maxX, minY, minZ) -> (maxX, maxY, minZ) -> (maxX, minY, maxZ)
	};

	// 填充 attrib_t 的顶点数据
	for (const auto& vertex : vertices) {
		aabbGeo.vertexAttrib.vertices.push_back(vertex.x);
		aabbGeo.vertexAttrib.vertices.push_back(vertex.y);
		aabbGeo.vertexAttrib.vertices.push_back(vertex.z);
		aabbGeo.vertexAttrib.colors.push_back(1);
		aabbGeo.vertexAttrib.colors.push_back(0);
		aabbGeo.vertexAttrib.colors.push_back(0);
	}

	// 填充 shape_t 的索引数据
	aabbGeo.shapes.resize(1);

	aabbGeo.shapes[0].mesh.indices.reserve(indices.size());
	aabbGeo.shapes[0].mesh.num_face_vertices.resize(12, 3);
	for (size_t i = 0; i < indices.size(); ++i) {
		tinyobj::index_t index;
		index.vertex_index = indices[i];
		index.normal_index = -1; // 没有法线数据
		index.texcoord_index = -1; // 没有纹理坐标数据
		aabbGeo.shapes[0].mesh.indices.push_back(index);
	}

}

void C21IntersectionTestMethodsExample::BuildSphereBVGeometry(Geometry& srcGeo, Geometry& sphereBVGeo)
{
	static const float M_PI = 3.141592653;

	struct Sphere {
		// 创建球面三角网格
		// center: 球的中心
		// radius: 球的半径
		// slices: 经度切片数量
		// stacks: 纬度切片数量
		static void CreateSphereMesh(const glm::vec3& center, float radius, int slices, int stacks,
			tinyobj::attrib_t& attrib, tinyobj::shape_t& shape) {
			// 存储球面顶点和索引数据
			std::vector<float> vertices;
			std::vector<unsigned int> indices;

			// 创建球面顶点
			for (int i = 0; i <= stacks; ++i) {
				float phi = M_PI * i / stacks;  // 纬度角，从0到π
				for (int j = 0; j <= slices; ++j) {
					float theta = 2 * M_PI * j / slices;  // 经度角，从0到2π

					// 计算球面上每个点的坐标
					float x = center[0] + radius * sin(phi) * cos(theta);
					float y = center[1] + radius * sin(phi) * sin(theta);
					float z = center[2] + radius * cos(phi);

					// 添加到顶点列表
					vertices.push_back(x);
					vertices.push_back(y);
					vertices.push_back(z);
					attrib.colors.push_back(0);
					attrib.colors.push_back(1);
					attrib.colors.push_back(0);
				}
			}

			// 创建球面三角形索引
			for (int i = 0; i < stacks; ++i) {
				for (int j = 0; j < slices; ++j) {
					int first = i * (slices + 1) + j;
					int second = first + slices + 1;

					// 第一三角形
					indices.push_back(first);
					indices.push_back(second);
					indices.push_back(first + 1);

					// 第二三角形
					indices.push_back(first + 1);
					indices.push_back(second);
					indices.push_back(second + 1);

				}
			}

			// 填充 tinyobj::attrib_t
			attrib.vertices = vertices;

			// 填充 tinyobj::shape_t
			shape.name = "Sphere";
			shape.mesh.indices.reserve(indices.size());
			shape.mesh.num_face_vertices.resize(indices.size() / 3,3);
			for (size_t i = 0; i < indices.size(); ++i) {
				tinyobj::index_t index;
				index.vertex_index = indices[i];
				index.normal_index = -1; // 没有法线数据
				index.texcoord_index = -1; // 没有纹理坐标数据
				shape.mesh.indices.push_back(index);
			}
		}
	};

	glm::vec3 sphereCenter = srcGeo.AABBcenter;



	Sphere sphere;
	sphereBVGeo.shapes.resize(1);
	
	sphere.CreateSphereMesh(sphereCenter, srcGeo.BVSphereRadius, 10, 10, sphereBVGeo.vertexAttrib, sphereBVGeo.shapes[0]);

	std::cout << "Create sphere bounding box geo" << std::endl;




}

bool C21IntersectionTestMethodsExample::IntersectRaySphere(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 sphereCenter, float radius, float& t)
{
	rayDirection = glm::normalize(rayDirection);
	
	glm::vec3 l = sphereCenter - rayOrigin;
	float r2 = glm::pow(radius, 2);//半径平方
	float l2 = pow(glm::length(l), 2);//射线原点到球心距离平方


	float cosTheta = glm::dot(glm::normalize(l), rayDirection);

	float s = glm::dot(l, rayDirection);//射线原点到球心向量在射线方向上的投影
	if (s < 0 && l2 >= r2)
	{
		return false;
		
	}
	float s2 = glm::pow(s, 2);
	float m2 = l2 - s2;//球心到射线d的距离平方
	if (m2 > r2)
	{
		return false;
	}

	float q2 = r2 - m2;//所求交点到球心和射线d之间垂线的距离平方
	float q = glm::sqrt(q2);
	t = s - q;
	return true;
}
