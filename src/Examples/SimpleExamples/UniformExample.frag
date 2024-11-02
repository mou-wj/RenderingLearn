#version 450 core
layout(set = 0,binding = 0,std140) uniform Buffer{
	vec2 windowSize;
	layout(offset = 8) bool enableTexture; 

};

layout(set = 0,binding = 1) uniform sampler2D testTexture;

layout(location = 0) out vec4 outColor;

void main(){
	vec2 uv = gl_PointCoord.xy / windowSize;
	if(enableTexture)
	{
		outColor = texture(testTexture,uv);

	}else {
		outColor = vec4(1.0,0.0,0.0, 1.0);
	}


}