#version 450
layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 0) uniform sampler2D baseColorTexture;
layout(input_attachment_index = 0,set = 0,binding = 3) uniform subpassInput inputAttachment;

layout(set = 0,binding = 2) uniform BlendInfo{
    float blendAlpha;  
};

void main(){
	vec3 baseColor = texture(baseColorTexture,inTexCoord).xyz;
	//gama ½âÂë
	baseColor = pow(baseColor, vec3(2.4));

	vec4 lastLodColor = subpassLoad(inputAttachment);
	//»ìºÏlod
	vec3 preLodColor = lastLodColor.xyz;
	vec3 color = preLodColor * (1 - blendAlpha) + baseColor * (blendAlpha);
	outColor = vec4(color, 1.0);
}