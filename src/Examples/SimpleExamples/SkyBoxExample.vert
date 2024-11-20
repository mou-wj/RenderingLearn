#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 in1;
layout(location = 2) in vec3 in2;
layout(location = 3) in vec3 in3;
layout(location = 4) in vec3 in4;
layout(location = 5) in vec3 in5;
layout(location = 0) out vec3 sampleVec;
layout(set = 0,binding = 0,std140) uniform Buffer{
	mat4 world;
	mat4 view;
	mat4 proj;
};
layout(location  =1) out vec3 color;
void main(){
	//天空盒的模型不需要位移，只需要旋转就行
	mat4 viewRotate = mat4(mat3(view));
	gl_Position = proj * view * world *vec4(inPosition,1.0f);
	gl_Position = proj * viewRotate *vec4(inPosition,1.0f);
	gl_Position= gl_Position.xyww;//
	sampleVec = normalize(inPosition);
	if(inPosition.x >0)
	{
	color = vec3(1.0,0.0,0.0);
	}else {
	color = vec3(0.0,0.0,1.0);
	}
}