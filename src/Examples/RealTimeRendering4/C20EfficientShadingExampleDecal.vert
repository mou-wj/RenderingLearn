#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 in1;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(set = 0,binding = 0) uniform TransformParallel{
    mat4 transform;//mvp matrix ,平行投影
   
};
void main(){
	gl_Position =transform * vec4(inPosition,1.0f);
	outPosition = inPosition;
	gl_Position/=gl_Position.w;
	outUV = (gl_Position.xy + 1)/2;
}