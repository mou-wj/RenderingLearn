#version 450 core

layout(location = 0) out vec4 outColor;


void main(){
	// ��ȡ��ǰƬ�ε����ֵ
    float depth = gl_FragCoord.z;
	
	outColor = vec4(depth,0,0,1);
	


}