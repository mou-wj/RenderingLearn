#version 450 core

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput myInputAttachment;

void main(){

    vec4 color = subpassLoad(myInputAttachment); // 读取输入附件

	const vec3 frogColor = vec3(1,1,1);
	float alpha = color.w;
	//混合雾
	alpha = clamp(alpha,0,0.9);
	vec3 blendColor = (1-alpha) * color.xyz + alpha * frogColor;


	outColor = vec4(blendColor,1.0);
}