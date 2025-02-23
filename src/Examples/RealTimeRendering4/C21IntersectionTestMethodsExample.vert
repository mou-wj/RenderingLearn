#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 in1;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 in3;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 outColor;

layout(set = 0,binding = 0) uniform Transform{
    mat4 transform;//mvp matrix
   
};

void main(){
	gl_Position =transform * vec4(inPosition,1.0f);
	outColor = inColor;
}