#version 450 core
layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

void main(){
	gl_FragDepth = gl_FragCoord.z - 0.0001; //ֱ�ӽ��������ȼ�ȥһ��ƫ�������Ӷ����Ƶ�����֮��
	outColor = vec4(0.2,0,0,1.0);

}