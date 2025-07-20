#version 450 core
layout(set = 0,binding = 0,std140) uniform Buffer{
	vec2 windowSize;
	bool enableTexture; 

};

layout(set = 0,binding = 1) uniform sampler2D testTexture;

layout(location = 0) out vec4 outColor;

void main(){
	vec2 uv = gl_FragCoord.xy / windowSize;
	if(enableTexture)
	{
		vec3 texColor = texture(testTexture,uv).xyz;

		//gama ½âÂë
		texColor = pow(texColor, vec3(2.4));

		outColor = vec4(texColor,1.0);
//		outColor = vec4(uv,0.0,1.0);
	}else {
		outColor = vec4(1.0,0.0,0.0, 1.0);
	}


}