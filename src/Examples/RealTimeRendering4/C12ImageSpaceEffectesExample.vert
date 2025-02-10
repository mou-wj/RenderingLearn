#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec2 curUV;
void main(){
	gl_Position = vec4(inPosition,1.0f);
	gl_Position /=gl_Position.w;
	curUV = (gl_Position.xy +1)/2;


}