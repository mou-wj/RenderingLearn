#version 450 core
layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor;

void main(){
	gl_FragDepth = gl_FragCoord.z - 0.0001;
	outColor = vec4(1,0,1,1.0);

}