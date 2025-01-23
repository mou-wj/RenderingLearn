#version 450 core

layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;


//各项同性的GGX PDF
float NDF_GGX_Isotropy(float nDotM,float roughness){
	float res = 0;
	float numerator = max(nDotM,0) * pow(roughness,2);
	float denominator = s_pi * pow( 1 + pow(nDotM,2) * (pow(roughness,2) - 1) ,2);

	res = numerator / denominator;
	return res;
}

//shadow和mask函数
float G2Smith(float mDotV,float mDotL,float lambdaV/*v视角的lambda函数结果*/,float lambdaL/*l视角的lambda函数结果*/){
	mDotV = max(mDotV,0);
	mDotL = max(mDotL,0);
	float res = mDotV * mDotL / ( 1 + lambdaV + lambdaL);
	return res;
}

//各项同性的GGX Lambda函数
float LambdaGGX_Isotropy(float nDotS,float roughness){
	float res = 0;
	float a = nDotS / (roughness * sqrt( 1 - pow(nDotS,2) ));
	res = (sqrt(1 + 1 / pow(a,2)) - 1) / 2; 
	return res;


}




//预计算方法不对，先暂时搁置，后续再做
void PreComputeMultiReflectDirectionalAlbedo(){
	//根据当前位置得到粗糙度以及cos值
	vec2 uv = gl_FragCoord.xy / vec2(32);
	float roughness = uv.x;
	float nDotV = uv.y;//

	//计算向量V
	vec3 V;
	V.x = 0;
	V.y = -nDotV;
	V.z = sqrt(1 - pow(nDotV,2));


	vec3 n = vec3(0,-1,0);

	//开始预计算
	uint numThetaStep = 32;
	uint numPhiStep = 32;




	float deltaPhi = (2 * s_pi) / numPhiStep;
	float deptaTheta = 0.5 * s_pi / numThetaStep;

	float total = 0;
	for(uint thetaIndex = 0;thetaIndex < numThetaStep;thetaIndex++)
	{
		for(uint phiIndex = 0;phiIndex < numPhiStep;phiIndex++)
		{
			float curTheta = deptaTheta* thetaIndex;
			float curPhi = deltaPhi * phiIndex;
			//计算另外一个向量
			vec3 L ;
			L.x = sin(curTheta) * cos(curPhi);
			L.z = sin(curTheta) * sin(curPhi);
			L.y = -cos(curTheta);



			vec3 h = normalize((L + V)/2);


			float hDotL = dot(h,L);
			float hDotV = dot(h,V);

			float nDotL = dot(L,n);
			float nDotV = dot(V,n);

			float lambdaV = LambdaGGX_Isotropy(hDotV,roughness);
			float lambdaL = LambdaGGX_Isotropy(hDotL,roughness);
	

			float ndf_ggx = NDF_GGX_Isotropy(dot(vec3(0,-1,0),h),roughness);
			float g2Smith = G2Smith(hDotV,hDotL,lambdaV,lambdaL);

			float frenel = 1.0;//预计算让Frenel项为1

			float f_spec = frenel * g2Smith * ndf_ggx * max(hDotL,0) * max(hDotV,0) / ( 4 * abs(nDotL) * abs(nDotV) );

			float res = f_spec * max(hDotL,0) * sin(curTheta) * deltaPhi * deptaTheta;//积分项

			total+=res;//积分
		
		}
	
	}

	outColor.y = total;








}



void main(){
	outColor = vec4(1,0,0,1);
	PreComputeMultiReflectDirectionalAlbedo();
}