#version 450 core

layout(location = 0) out vec4 outColor;
layout(set = 0,binding = 1,std140) uniform SimpleSceenExampleBuffer{
	float cur;
};

void main(){
	// ��ȡ��ǰƬ�ε����ֵ
    float depth = gl_FragCoord.z;
	
	outColor = vec4(cur,0,0,1);
	


}