#version 450 core

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的法线
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

layout(set = 0,binding = 1,std140) uniform Info{
	layout(offset = 0) vec3 cameraPos;//世界空间中相机位置
};


layout(set = 0,binding = 2) uniform sampler2D baseColorTexture;

layout(set = 0,binding = 3) uniform sampler2D roughnessTexture;
layout(set = 0,binding = 4) uniform sampler2D normalTexture;
//layout(set = 0,binding = 5) uniform sampler2D displacementTexture;
layout(set = 0,binding = 6) uniform samplerCube bgEnvTexture;


// 生成Hammersley 序列的一维采样点，各个维度的序列可以使用不同的基来生成
float HammersleySequence(uint index, uint base) {
    float result = 0.0;
    float f = 1.0;
    uint i = index;
    while (i > 0) {
        f /= float(base);
        result += f * float(i % base);
        i /= base;
    }
    return result;
}

// 生成均匀分布的二维Halton采样点
vec2 HaltonSample2D(uint index) {
    return vec2(HammersleySequence(index, 2),HammersleySequence(index, 3)); // 基数选择 2,3
}



//各项同性的GGX PDF
float NDF_GGX_Isotropy(float nDotM,float roughness){
	float res = 0;
	float numerator = max(nDotM,0) * pow(roughness,2);
	float denominator = s_pi * pow( 1 + pow(nDotM,2) * (pow(roughness,2) - 1) ,2);

	res = numerator / denominator;
	return res;
}

//各项同性的GGX Lambda函数
float LambdaGGX_Isotropy(float nDotS,float roughness){
	float res = 0;
	float a = nDotS / (roughness * sqrt( 1 - pow(nDotS,2) ));
	res = (sqrt(1 + 1 / pow(a,2)) - 1) / 2; 
	return res;


}




//mask函数
float G1(vec3 m,vec3 v,float lambdaV/*v视角的lambda函数结果*/){
	float mDotV = max(dot(m,v),0);
	float res = mDotV / ( 1 + lambdaV);
	return res;
}

//shadow和mask函数
float G2Smith(float mDotV,float mDotL,float lambdaV/*v视角的lambda函数结果*/,float lambdaL/*l视角的lambda函数结果*/){
	mDotV = max(mDotV,0);
	mDotL = max(mDotL,0);
	float res = mDotV * mDotL / ( 1 + lambdaV + lambdaL);
	return res;
}

vec3 CaculateLamberDiffuseColor(vec3 n){
	vec3 res = vec3(0);
	float curPhi =0,curTheta = 0;
	float lambertBRDF = 1 / s_pi;
	vec3 l = vec3(0);
	vec3 sampleColor = vec3(0);

	float nDotL = 0;

	uint numSample = 40;
	vec2 samplePoint;
	float delta = 2 * s_pi/ numSample;


			//根据世界空间的法线构建上半球坐标系
	vec3 y = -normalize(n);
	vec3 x,z;
	if((1 - abs(y.x)) > 0.00000001)
	{
		x = vec3(1,0,0);
	}else {
		x = vec3(0,1,0);
	}
	z = normalize(cross(x,y));
	x = normalize(cross(y,z));
	mat3 nornalMatrix = mat3(x,y,z);

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		samplePoint = HaltonSample2D(sampleIndex); 
		curPhi = samplePoint.x * 0.5 * s_pi;
		curTheta = samplePoint.y * 2 * s_pi;


		l.y = -cos(curPhi);
		l.x = sin(curPhi) * cos(curTheta);
		l.z = sin(curPhi) * sin(curTheta);

		l = nornalMatrix * l;//变换到世界坐标系中的向量

		sampleColor = texture(bgEnvTexture,l).xyz;
		//sampleColor = pow(sampleColor, vec3(2.4));

		nDotL = max(dot(l,n),0);

		res += lambertBRDF * sampleColor * nDotL *  delta;

	
	}
	return res;	
}

//根据一个符合均匀分布的各项范围在-1到1之间的样本，拟合变换得到一个近似微平面反射光的方向和完美反射光之间的theta和fine差值的分布结果，返回值第一项为theta的差值，范围-pi到pi，第二项为fine的差值，范围-pi/2 到pi/2
vec2 FittingInverseCDFSparrowReflect(vec2 daltaXY/*两个分别为-1到1之间的二维参数*/){
	vec2 res;
	//第一项表示和目标向量在theta角的差值，范围-pi到pi
	res.x = asin(pow(daltaXY.x,3)) * 2;
	res.y = asin(pow(daltaXY.y,3));


	return res;
}


float Frenel(float nDotL,float F0){
	float r = F0 + (1- F0) * pow(1 - max(nDotL,0),5);
	return r;

}

vec3 CaculateGGXSpecularColor(vec3 n,float roughness){
	vec3 res = vec3(0);
	vec3 v = normalize(cameraPos - inPosition);
	vec3 l;

	float curPhi =0,curTheta = 0;
	vec3 sampleColor = vec3(0);

	float nDotL = 0;

	uint numSample = 4;
	vec2 samplePoint;
	float delta = 2 * s_pi/ numSample;
		//根据世界空间的法线构建上半球坐标系
	vec3 y = -normalize(n);
	vec3 x,z;
	if((1 - abs(y.x)) > 0.00000001)
	{
		x = vec3(1,0,0);
	}else {
		x = vec3(0,1,0);
	}
	z = normalize(cross(x,y));
	x = normalize(cross(y,z));
	mat3 nornalMatrix = mat3(x,y,z);
	mat3 normalMatrixInverse = inverse(nornalMatrix);

		//将反射向量变换到法向量的本地坐标系中
	vec3 wr = reflect(-v,n);
	wr = normalize(normalMatrixInverse * wr);
	//计算wr在本地坐标系中的theta值和fine值
	float theta_r = atan(wr.z,wr.x);
	if(theta_r <0)
	{
		theta_r+=2* s_pi;
	}
	float fine_r = acos(-wr.y) ; 
	float totalW = 0;

	for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++)
	{
		samplePoint = HaltonSample2D(sampleIndex); 
		curPhi = samplePoint.x * 0.5 * s_pi;
		curTheta = samplePoint.y * 2 * s_pi;


		samplePoint -=0.5;
		samplePoint *=2;//变到-1到1之间
		vec2 deltaThetaFine = FittingInverseCDFSparrowReflect(samplePoint);
			
		//根据反射向量的theta和fine值以及变换后的样本点和反射向量theta和fine的差值获得样本向量的theta和fine值
		curTheta = deltaThetaFine.x + theta_r;
		curPhi = deltaThetaFine.y + fine_r;


		

		l.y = -cos(curPhi);
		l.x = sin(curPhi) * cos(curTheta);
		l.z = sin(curPhi) * sin(curTheta);

		
		l = nornalMatrix * l;//变换到世界坐标系中的向量




		vec3 h = normalize((l + v) / 2);
		float hDotL = dot(h,l);
		float hDotV = dot(h,v);
		float nDotL = dot(l,n);
		float nDotV = dot(v,n);

		float lambdaV = LambdaGGX_Isotropy(hDotV,roughness);
		float lambdaL = LambdaGGX_Isotropy(hDotL,roughness);


		float ndf_ggx = NDF_GGX_Isotropy(dot(n,h),0.1);
		float frenel =Frenel(nDotL,0.04);
		float g2Smith = G2Smith(hDotV,hDotL,lambdaV,lambdaL);

		float f_spec = frenel * g2Smith * ndf_ggx / ( 4 * abs(nDotL) * abs(nDotV) );

		sampleColor = texture(bgEnvTexture,l).xyz;
		sampleColor = pow(sampleColor, vec3(2.4));

		vec3 specColor = f_spec * sampleColor * max(hDotL,0) * delta;
		res += specColor;
	}
	return res;	
}



void main(){
	outColor = vec4(1,0,0,1);
	
	vec3 baseColor = texture(baseColorTexture,inTexCoord.xy).xyz;
	baseColor = pow(baseColor,vec3(2.4));

	float roughness = texture(roughnessTexture,inTexCoord.xy).x;
	vec3 n = texture(normalTexture,inTexCoord.xy).xyz;
	
	//将采样的法线变换到世界坐标
	vec3 textureNormal = normalize(cross(inTan,inBiTan));
	mat3 texCoordSpaceMat = mat3(inTan,textureNormal,inBiTan);

	n = normalize(n.x * inTan + n.y * inBiTan + n.z * textureNormal);
	float kd = 0.8,ks = 1 - kd;

	vec3 diffuse = CaculateLamberDiffuseColor(n);
	vec3 spec = CaculateGGXSpecularColor(n,roughness);


	vec3 color =kd * diffuse * baseColor + spec * ks;

	outColor = vec4(color,1);
	//outColor = vec4(kd * diffuse * baseColor,1);
	//outColor = vec4(spec,1.0);




}