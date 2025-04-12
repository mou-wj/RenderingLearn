#version 450 core

layout(location = 0) in vec3 inTexCoord;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inPosition1;
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 2) uniform sampler2D baseColorTexture;
void main(){
	vec3 sampleColor = texture(baseColorTexture,inTexCoord.xy * vec2(1,-1)).xyz;
	sampleColor = pow(sampleColor,vec3(2.2));
	outColor = vec4(1,0,0,1);
	outColor = vec4(gl_FragCoord.z,sampleColor);
	//outColor = vec4(gl_FragCoord.z,0,0,1.0);
}