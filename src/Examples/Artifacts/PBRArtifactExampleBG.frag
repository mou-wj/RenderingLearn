#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 outColor;

layout(set =0,binding = 1) uniform samplerCube bgEnvTexture;





void main(){
	outColor = vec4(1,0,0,1);
	vec4 sampleColor = texture(bgEnvTexture,normalize(inPosition));
	//
	sampleColor = pow(sampleColor, vec4(2.4));
	outColor = vec4(sampleColor.xyz,1.0);

}