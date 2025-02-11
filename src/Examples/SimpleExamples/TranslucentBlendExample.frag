#version 450 core
layout(set = 0,binding = 0,std140) uniform Buffer{
	vec4 color;

};

layout(set = 0,binding = 1) uniform sampler2D testTexture;

layout(location = 0) in vec3 inColor;

layout(location = 0) out vec4 outColor;

void main(){
	outColor = vec4(inColor, 0.5);


}