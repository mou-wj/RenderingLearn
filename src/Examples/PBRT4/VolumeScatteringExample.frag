#version 450 core


layout(location = 0) in vec3 inSampleVec;
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

//phase function
float PhaseFunctionConstant(){
	return 1 / ( 4 * s_pi);
}

//Henyey–Greenstein Phase Function
float PhaseFunctionHenyeyGreenstein(float cosTheta, float g){
    float denom = 1 + pow(g,2) + 2 * g * cosTheta;
	denom = pow(denom,1.5) * 4 * s_pi;
	return (1 - pow(g,2)) / denom;
}



void main(){

//场景所有对象绘制为白色
	outColor = vec4(1,1,1,1);

}