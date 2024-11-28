#version 450 core
layout(set = 0,binding = 0,std140) uniform Buffer{
	vec2 windowSize;

};

//这里直接采样显示就可以
layout(set = 0,binding = 1) uniform sampler2D testTexture;

layout(location = 0) out vec4 outColor;







void main(){
	vec2 uv = gl_FragCoord.xy / windowSize;

	vec4 texColor = texture(testTexture,uv);
	outColor = texColor;



}