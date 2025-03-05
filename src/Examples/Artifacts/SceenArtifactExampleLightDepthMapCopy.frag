#version 450 core

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput myInputAttachment;
layout(rgba32f,set = 0,binding = 1) uniform image2D depthTexture;
void main(){

    vec4 color = subpassLoad(myInputAttachment); // 读取输入附件
    // 获取纹理尺寸
    ivec2 size = imageSize(depthTexture);
	ivec2 coord = ivec2(size * inUV);
	imageStore(depthTexture,coord,color);


	outColor = color;
}