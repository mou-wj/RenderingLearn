#version 450 core
layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 1) uniform IntersectInfo{
    bool isIntersect;//mvp matrix
   
};

void main(){
	outColor = vec4(inColor, 1.0);
	if(isIntersect)
	{
		outColor = vec4(0,0,1,1);
	}
}