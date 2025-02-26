#include "C22IntersectionTestMethodsExample.h"

void C22IntersectionTestMethodsExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C22IntersectionTestMethodsExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C22IntersectionTestMethodsExample.frag";
	
	ShaderCodePaths BVMeshCodePath;
	BVMeshCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C22IntersectionTestMethodsExample.vert";
	BVMeshCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C22IntersectionTestMethodsExampleBVMesh.frag";

	ShaderCodePaths pickTrianbleCodePath;
	pickTrianbleCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C22IntersectionTestMethodsExample.vert";
	pickTrianbleCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C22IntersectionTestMethodsExamplePickTriangle.frag";


	renderPassInfos.resize(1);
	renderPassInfos[0].InitDefaultRenderPassInfo({ shaderCodePath,BVMeshCodePath,pickTrianbleCodePath }, windowWidth, windowHeight);


	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件
	
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件

	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件
}

void C22IntersectionTestMethodsExample::InitResourceInfos()
{

	
	geoms.resize(4);

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

void C22IntersectionTestMethodsExample::Loop()
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
		float normalizeY = (pos[1] - halfH)/halfH;

		glm::vec3 rayOrigin, rayDirection;
		camera.GenerateRay(glm::vec2(normalizeX,normalizeY), rayOrigin, rayDirection);
		float t = 0;
		auto intersect = IntersectRaySphere(rayOrigin, rayDirection, geoms[0].AABBcenter, geoms[0].BVSphereRadius, t);
		
		auto intersectAABB = IntersectRayAABB(rayOrigin, rayDirection, geoms[0].AABBs, t);
		FillBuffer(buffers["IntersectInfo"], 0, sizeof(bool), (const char*)&intersectAABB);

		bool pickSuccess = PickTriangleFromGeom(geoms[0], rayOrigin, rayDirection, geoms[3]);

		if (pickSuccess)
		{
			renderPassInfos[0].subpassDrawGeoInfos[2] = { 3 };
			WaitIdle();//等待队列执行完
			ReInitGeometryResources(geoms[3]);
		}
		else {
			renderPassInfos[0].subpassDrawGeoInfos[2] = {};
		}
		
		}, "鼠标点击屏幕发射一条射线，如果和边界盒相交，其对应边界的几何网格变为蓝色，如果和模型上的三角形相交，则显示该三角形为紫色");


	BindBuffer("Transform");
	bufferBindInfos["Transform"].pipeId = 1;
	BindBuffer("Transform");
	bufferBindInfos["Transform"].pipeId = 2;
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

		glm::mat4 transform = camera.GetProj() * camera.GetView();//不进行世界坐标变换
		FillBuffer(buffers["Transform"], 0, sizeof(glm::mat4), (const char*)&transform);
		CmdListWaitFinish(graphicCommandList);
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);
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

void C22IntersectionTestMethodsExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}

void C22IntersectionTestMethodsExample::BuildAABBGeometry(Geometry& srcGeo, Geometry& aabbGeo)
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

void C22IntersectionTestMethodsExample::BuildSphereBVGeometry(Geometry& srcGeo, Geometry& sphereBVGeo)
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

bool C22IntersectionTestMethodsExample::IntersectRaySphere(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 sphereCenter, float radius, float& t)
{
	rayDirection = glm::normalize(rayDirection);
	
	glm::vec3 l = sphereCenter - rayOrigin;
	float r2 = glm::pow(radius, 2);//半径平方
	float l2 = pow(glm::length(l), 2);//射线原点到球心距离平方



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

bool C22IntersectionTestMethodsExample::IntersectRayAABB(glm::vec3 rayOrigin, glm::vec3 rayDirection, Geometry::AABB& aabb, float& t)
{
	rayDirection = glm::normalize(rayDirection);//归一化

	// Initialize tMin and tMax as the extreme values
	float tMin = -INFINITY;
	float tMax = INFINITY;

	// Check intersection along the X axis
	if (rayDirection.x != 0) {
		float t1 = (aabb.minX - rayOrigin.x) / rayDirection.x;
		float t2 = (aabb.maxX - rayOrigin.x) / rayDirection.x;
		if (t1 > t2) std::swap(t1, t2);  // Ensure t1 <= t2

		tMin = std::max(tMin, t1);
		tMax = std::min(tMax, t2);

		// If tMin > tMax, no intersection
		if (tMin > tMax) return false;
	}
	else {
		// If rayDirection.x is 0, check if ray is inside the AABB along the x-axis
		if (rayOrigin.x < aabb.minX || rayOrigin.x > aabb.maxX) {
			return false;  // No intersection
		}
	}

	// Check intersection along the Y axis
	if (rayDirection.y != 0) {
		float t1 = (aabb.minY - rayOrigin.y) / rayDirection.y;
		float t2 = (aabb.maxY - rayOrigin.y) / rayDirection.y;
		if (t1 > t2) std::swap(t1, t2);  // Ensure t1 <= t2

		tMin = std::max(tMin, t1);
		tMax = std::min(tMax, t2);

		// If tMin > tMax, no intersection
		if (tMin > tMax) return false;
	}
	else {
		// If rayDirection.y is 0, check if ray is inside the AABB along the y-axis
		if (rayOrigin.y < aabb.minY || rayOrigin.y > aabb.maxY) {
			return false;  // No intersection
		}
	}

	// Check intersection along the Z axis
	if (rayDirection.z != 0) {
		float t1 = (aabb.minZ - rayOrigin.z) / rayDirection.z;
		float t2 = (aabb.maxZ - rayOrigin.z) / rayDirection.z;
		if (t1 > t2) std::swap(t1, t2);  // Ensure t1 <= t2

		tMin = std::max(tMin, t1);
		tMax = std::min(tMax, t2);

		// If tMin > tMax, no intersection
		if (tMin > tMax) return false;
	}
	else {
		// If rayDirection.z is 0, check if ray is inside the AABB along the z-axis
		if (rayOrigin.z < aabb.minZ || rayOrigin.z > aabb.maxZ) {
			return false;  // No intersection
		}
	}

	// If tMin is greater than or equal to zero, we have a valid intersection
	if (tMin >= 0) {
		t = tMin;
		return true;
	}

	// If tMax is greater than or equal to zero, we have a valid intersection
	if (tMax >= 0) {
		t = tMax;
		return true;
	}

	return false; // No intersection
}

bool C22IntersectionTestMethodsExample::IntersectRayTriangle(glm::vec3 rayOrigin, glm::vec3 rayDirection, std::array<glm::vec3, 3>& trianglePoints, float& t)
{
	glm::vec3 e1 = trianglePoints[1] - trianglePoints[0];
	glm::vec3 e2 = trianglePoints[2] - trianglePoints[0];
	glm::vec3 s = rayOrigin - trianglePoints[0];

	

	//计算行列式| -d  e1 e2 |
	float detNdE1E2 = glm::determinant(glm::mat3(-rayDirection, e1, e2));
	if (detNdE1E2 == 0)
	{
		return false;
	}
	float detSE1E2 = glm::determinant(glm::mat3(s, e1, e2));
	float detNdSE2 = glm::determinant(glm::mat3(-rayDirection, s, e2));
	float detNdE1S = glm::determinant(glm::mat3(-rayDirection, e1, s));

	glm::vec3 tuv = glm::vec3(detSE1E2, detNdSE2, detNdE1S) / detNdE1E2;
	if (tuv.y < 0 || tuv.z < 0 || tuv.y + tuv.z > 1)
	{
		return false;
	}


	t = tuv.x;
	
	if (t < 0)
	{
		return false;
	}
	
	return true;
}

bool C22IntersectionTestMethodsExample::IntersectTwoTriangle(std::array<glm::vec3, 3>& trianglePoints1, std::array<glm::vec3, 3>& trianglePoints2)
{
	std::array<glm::vec4, 3> triangle1 = { glm::vec4(trianglePoints1[0], 1.0) ,glm::vec4(trianglePoints1[1], 1.0) ,glm::vec4(trianglePoints1[2], 1.0) };
	std::array<glm::vec4, 3> triangle2 = { glm::vec4(trianglePoints2[0], 1.0) ,glm::vec4(trianglePoints2[1], 1.0) ,glm::vec4(trianglePoints2[2], 1.0) };
	//glm::vec4 p1 = glm::vec4(trianglePoints1[0], 1.0);
	//glm::vec4 p2 = glm::vec4(trianglePoints1[1], 1.0);
	//glm::vec4 p3 = glm::vec4(trianglePoints1[2], 1.0);
	//glm::vec4 q1 = glm::vec4(trianglePoints1[0], 1.0);
	//glm::vec4 q2 = glm::vec4(trianglePoints1[1], 1.0);
	//glm::vec4 q3 = glm::vec4(trianglePoints1[2], 1.0);
	
	auto det = [](glm::vec4 v1, glm::vec4 v2, glm::vec4 v3, glm::vec4 v4)->float {
		float res = glm::determinant(glm::mat4(v1, v2, v3, v4));
		return res;
		};
	
	//判断三角形是否和另外一个三角形所在的平面相交
	int flag1 = 0, flag2 = 0;

	std::array<float, 6> res;
	for (uint32_t i = 0; i < 3; i++)
	{
		float res1 = det(triangle1[0], triangle1[1], triangle1[2], triangle2[i]);
		float res2 = det(triangle2[0], triangle2[1], triangle2[2], triangle1[i]);

		if (res1 > 0)
		{
			flag1++;
		}
		else if (res1 < 0)
		{
			flag1+=10;
		}
		else {
			flag1 += 100;
		}
		if (res2 > 0)
		{
			flag2++;
		}
		else if (res2 < 0)
		{
			flag2+=10;

		}
		else {
			flag2 += 100;
		}
		res[i] = res1;
		res[3 + i] = res2;
	}

	if (flag1 == 3 || flag1 == 30 || flag2 == 3 || flag2 == 30) {
		return false;
	}


	//重排序

	for (uint32_t i = 0; i < 3; i++)
	{
		if ((flag1 == 102 || flag1 == 120) && res[i] == 0)
		{
			//将i号元素和0号元素交换
			std::swap(triangle1[0], triangle1[i]);
		}
		if (flag1 == 12 && res[i] < 0) {
			std::swap(triangle1[0], triangle1[i]);
		}
		if (flag1 == 21 && res[i] > 0) {
			std::swap(triangle1[0], triangle1[i]);
		}
		
		if ((flag2 == 102 || flag2 == 120) && res[i + 3] == 0)
		{
			//将i号元素和0号元素交换
			std::swap(triangle2[3], triangle2[i + 3]);
		}
		if (flag2 == 12 && res[i+3] < 0) {
			std::swap(triangle2[3], triangle2[i + 3]);
		}
		if (flag2 == 21 && res[i + 3] > 0) {
			std::swap(triangle2[3], triangle2[i + 3]);
		}

	}

	//计算两个三角形和两个所在平面交线的相交部分
	float det1 = det(triangle1[0], triangle1[1], triangle1[0], triangle2[1]);
	float det2 = det(triangle1[0], triangle1[2], triangle1[2], triangle2[0]);

	if (det1 <= 0 && det2 <= 0)
	{
		return true;
	}
	
	return false;
}

bool C22IntersectionTestMethodsExample::PickTriangleFromGeom(Geometry& srcGeo, glm::vec3 rayOrigin, glm::vec3 rayDirection, Geometry& dstPickTrianglesGeo)
{
	dstPickTrianglesGeo.vertexAttrib.vertices.clear();
	dstPickTrianglesGeo.shapes.resize(1);
	dstPickTrianglesGeo.shapes[0].mesh.indices.clear();
	float t = 0;
	std::array<glm::vec3, 3> triangle;
	uint32_t numIntersectTriangle = 0;
	uint32_t curIndex = 0;
	bool pickSuccess = false;
	for (auto shapeId = 0; shapeId < srcGeo.shapes.size(); shapeId++)
	{
		uint32_t numTriangles = srcGeo.shapes[shapeId].mesh.indices.size() / 3;
		for (uint32_t triangleId = 0; triangleId < numTriangles; triangleId++)
		{
			uint32_t vertexId1 = srcGeo.shapes[shapeId].mesh.indices[3 * triangleId].vertex_index;
			uint32_t vertexId2 = srcGeo.shapes[shapeId].mesh.indices[3 * triangleId + 1].vertex_index;
			uint32_t vertexId3 = srcGeo.shapes[shapeId].mesh.indices[3 * triangleId + 2].vertex_index;
			glm::vec3 trianglePoint1 = glm::vec3(srcGeo.vertexAttrib.vertices[3 * vertexId1], srcGeo.vertexAttrib.vertices[3 * vertexId1 + 1], srcGeo.vertexAttrib.vertices[3 * vertexId1 + 2]);
			glm::vec3 trianglePoint2 = glm::vec3(srcGeo.vertexAttrib.vertices[3 * vertexId2], srcGeo.vertexAttrib.vertices[3 * vertexId2 + 1], srcGeo.vertexAttrib.vertices[3 * vertexId2 + 2]);
			glm::vec3 trianglePoint3 = glm::vec3(srcGeo.vertexAttrib.vertices[3 * vertexId3], srcGeo.vertexAttrib.vertices[3 * vertexId3 + 1], srcGeo.vertexAttrib.vertices[3 * vertexId3 + 2]);

			triangle = { trianglePoint1 ,trianglePoint2 ,trianglePoint3 };
			auto intersect = IntersectRayTriangle(rayOrigin, rayDirection,triangle , t);

			if (intersect)
			{
				pickSuccess = true;
				numIntersectTriangle++;
				for (uint32_t i = 0; i < 3; i++)
				{
					dstPickTrianglesGeo.vertexAttrib.vertices.push_back(triangle[i][0]);
					dstPickTrianglesGeo.vertexAttrib.vertices.push_back(triangle[i][1]);
					dstPickTrianglesGeo.vertexAttrib.vertices.push_back(triangle[i][2]);
					dstPickTrianglesGeo.shapes[0].mesh.indices.push_back({int(curIndex++)});
				}
			}


		}



	}


	dstPickTrianglesGeo.shapes[0].mesh.num_face_vertices.resize(numIntersectTriangle, 3);
	
	return pickSuccess;
}
