#version 450 core
layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 0) uniform sampler2D baseColorTexture;

void main(){
	vec3 baseColor = texture(baseColorTexture,inTexCoord).xyz;
			//gama ½âÂë
	baseColor = pow(baseColor, vec3(2.4));
	outColor = vec4(baseColor, 1.0);

}