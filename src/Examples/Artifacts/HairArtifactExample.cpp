#include "HairArtifactExample.h"
#include "tiny_gltf.h"
#include "Framework/Utils/ModelFileTool.h"

struct alignas(16) TransBuffer {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
} tbuffer;

struct alignas(16) ShadeBuffer {
	glm::vec4 baseColorFactor;
	glm::vec2 metalicAndRouguness;
	glm::vec3 pointLightPos;
	glm::vec3 pointLightColor;
	glm::vec3 viewPos;
} sbuffer;

void HairArtifactExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/HairArtifactExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/HairArtifactExample.frag";
	InitDefaultGraphicSubpassInfo(shaderCodePath);


}

void HairArtifactExample::InitResourceInfos()
{

	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(1);
	auto& geom = geoms[0];
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	

	auto hairModelSrc = std::string(PROJECT_DIR) + "/resources/glb/short_wavy_hair_with_bangs_with_bones.glb";
	RenderScene renderScene;
	LoadRenderScene(hairModelSrc, renderScene);
	geom.InitFromRenderScene(renderScene, {
		{Geometry::EAT_POSITION},
		{Geometry::EAT_NORMAL},
		{Geometry::EAT_TANGENT,VK_FORMAT_R32G32B32A32_SFLOAT},
		{Geometry::EAT_TEXTURE_COORDINATES,VK_FORMAT_R32G32_SFLOAT},
		});
	//	
	TextureDataSource dataSource;
	dataSource.imagePixelDatas.assign(renderScene.textures[0].pixels.begin(), renderScene.textures[0].pixels.end());
	dataSource.width = renderScene.textures[0].width;
	dataSource.height = renderScene.textures[0].height;

	textureBindInfos["baseColorTexture"].textureDataSources.push_back(dataSource);
	textureBindInfos["baseColorTexture"].binding = 1;
	textureBindInfos["baseColorTexture"].format = VK_FORMAT_R8G8B8A8_UNORM;

	dataSource.imagePixelDatas.assign(renderScene.textures[1].pixels.begin(), renderScene.textures[1].pixels.end());
	dataSource.width = renderScene.textures[1].width;
	dataSource.height = renderScene.textures[1].height;

	textureBindInfos["normalTexture"].textureDataSources.push_back(dataSource);
	textureBindInfos["normalTexture"].binding = 2;
	textureBindInfos["normalTexture"].format = VK_FORMAT_R8G8B8A8_UNORM;

	bufferBindInfos["TransBuffer"].size = sizeof(TransBuffer);
	bufferBindInfos["TransBuffer"].binding = 0;
	bufferBindInfos["ShadeBuffer"].size = sizeof(ShadeBuffer);
	bufferBindInfos["ShadeBuffer"].binding = 3;
}

void HairArtifactExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(6);
	Camera camera(glm::vec3(0, 0, -3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	TransBuffer tbuffer;
	tbuffer.model = glm::mat4(1.0);
	tbuffer.view = camera.GetView();
	tbuffer.proj = camera.GetProj();
	ShadeBuffer sbuffer;
	sbuffer.baseColorFactor = glm::vec4(1);
	sbuffer.metalicAndRouguness = glm::vec2(0);
	sbuffer.pointLightColor = glm::vec3(1, 0, 0);
	sbuffer.pointLightPos = glm::vec3(0, -5, 0);
	sbuffer.viewPos = camera.GetPos();

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
	
	FillBuffer(buffers["TransBuffer"], 0, sizeof(TransBuffer), (const char*)&tbuffer);
	FillBuffer(buffers["ShadeBuffer"], 0, sizeof(ShadeBuffer), (const char*)&sbuffer);

	auto drawFinished = semaphores[0];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = {  };
	submitSyncInfo.waitStages = {  };
	submitSyncInfo.sigSemaphores = { drawFinished };

	BindBuffer("TransBuffer");
	BindBuffer("ShadeBuffer");
	BindTexture("baseColorTexture");
	BindTexture("normalTexture");

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发
		tbuffer.view = camera.GetView();
		FillBuffer(buffers["TransBuffer"], 0, sizeof(TransBuffer), (const char*)&tbuffer);

		CmdListWaitFinish(graphicCommandList);
		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		PresentPassResult(drawFinished,0, 0);
		//

	}

}

void HairArtifactExample::InitSyncObjectNumInfo()
{
	//numFences = 1;
	numSemaphores = 2;
}

void HairArtifactExample::DrawImGui()
{
	// 你的UI内容
	ImGui::Begin("Hello Window");
	ImGui::Text("Hello, Vulkan ImGui!");
	ImGui::End();

}
