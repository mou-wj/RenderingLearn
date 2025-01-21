#version 450 core

layout(location = 0) in vec3 inViewWorldPosition;

layout(location = 0) out vec4 depthMapColor;


void main(){
	// 获取当前片段的深度值
    float depth = gl_FragCoord.z;

	float dist = distance(vec3(0,0,0),inViewWorldPosition);
	float dist2  =pow(dist,2);
	
	depthMapColor = vec4(depth,dist,dist2,1);
	




}