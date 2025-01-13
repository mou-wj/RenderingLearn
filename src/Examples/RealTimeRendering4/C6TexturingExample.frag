#version 450 core

//遗留问题：
//SAT采样返回全0的问题
//法线贴图正确性验证
//视差贴图
//纹理光源

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的位置
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 1) uniform sampler2D pic;
// 声明绑定的图像
layout(set = 0,binding = 2, rgba32f) readonly uniform image2D myImage;


layout(set = 0,binding = 3,std140) uniform Info{
	float animateDelta;
	layout(offset = 16) vec3 cameraPos;//世界空间中相机位置
	layout(offset = 32) int exampleType;//要显示的实例类型，0: 直接获取纹素值 ; 1:mipmap ;2: SAT; 3:随机生成的纹理;4:纹理动画;5:材质纹理;6:alpha贴图
};

layout(set = 0,binding = 4) uniform sampler2D alphaPic;
layout(set = 0,binding = 5) uniform sampler2D normalPic;
layout(set = 0,binding = 6) uniform sampler2D materialPic;

void RawFetch(){
	ivec2 size = textureSize(pic,0);
	vec3 sampleColor = texelFetch(pic,ivec2(inTexCoord.xy * size),0).xyz;
	outColor = vec4(sampleColor,1);

}

void MipMapExample(){
	float depth = gl_FragCoord.z; // 当前片段的深度值
	float deltaD = 1 / 8;//假设最大8个mipmap等级

	uint curMipLevel = uint(depth / deltaD);

	vec3 sampleColor = textureLod(pic,inTexCoord.xy,curMipLevel).xyz;
	outColor = vec4(sampleColor,1);

}

void SATExample(){
	
	ivec2 size = textureSize(pic,0);
	
	    // 计算当前片段的纹理坐标偏导数
	vec2 fragTexCoord = inTexCoord.xy;
    vec2 dx = dFdx(fragTexCoord);
    vec2 dy = dFdy(fragTexCoord);

    // 估算当前片段在纹理中的覆盖范围
    vec2 uvMin = fragTexCoord - 0.5 * abs(dx) - 0.5 * abs(dy);
    vec2 uvMax = fragTexCoord + 0.5 * abs(dx) + 0.5 * abs(dy);

	// 将 uvMin 和 uvMax 转换为纹理空间的像素坐标
    ivec2 texMin = ivec2(uvMin * size);
    ivec2 texMax = ivec2(uvMax * size);

	int deltaU = texMax.x-texMin.x;
	int deltaV = texMax.y-texMin.y;
	int numTexels = deltaU * deltaV;

	// 左上、右上、左下、右下四个整数坐标
    ivec2 topLeft = texMin;
    ivec2 topRight = ivec2(texMax.x, texMin.y);
    ivec2 bottomLeft = ivec2(texMin.x, texMax.y);
    ivec2 bottomRight = texMax;


	//这里获取的值存在为0的情况，如果正确不会出现为0的情况，可能是格式的问题,也有可能是填充image数据的时候出了问题，先暂时搁置
	//vec3 sumColorTopLeft = texelFetch(pic, topLeft, 0).xyz;
    //vec3 sumColorTopRight = texelFetch(pic, topRight, 0).xyz;
    //vec3 sumColorBottomLeft = texelFetch(pic, bottomLeft, 0).xyz;
    //vec3 sumColorBottomRight = texelFetch(pic, bottomRight, 0).xyz;
	vec3 sumColorTopLeft = imageLoad(myImage, topLeft).xyz;
    vec3 sumColorTopRight = imageLoad(myImage, topRight).xyz;
    vec3 sumColorBottomLeft = imageLoad(myImage, bottomLeft).xyz;
    vec3 sumColorBottomRight = imageLoad(myImage, bottomRight).xyz;

    // 计算总的 texel 数量
	vec3 color = sumColorBottomRight - sumColorTopRight - sumColorBottomLeft + sumColorTopLeft;

	if(numTexels >0)
	{
		color /=numTexels;
	}

	

	outColor = vec4(color,1);

}

float SimpleRandom(float seed,float fac){
	return (sin(seed * 444000.766) + 1)/2.0;

}

vec3 ProcessTexture(vec2 uv){
	float rx = SimpleRandom(uv.x,444000.766);
	float ry = SimpleRandom(uv.x,3332.998);
	return vec3(rx,ry,0);
}

//直接通过计算得到的纹理值
void ProcessTextureExample(){
	vec3 color = ProcessTexture(inTexCoord.xy);
	outColor = vec4(color,1);

}

//纹理动画，通过改变每一帧的uv值来实现纹理动画
void TextureAnimateExample(){
	vec2 uv = inTexCoord.xy;
	uv.y += animateDelta;
	if(uv.y > 1){
		uv.y -= 1;
	
	}

	ivec2 size = textureSize(pic,0);
	vec3 sampleColor = texelFetch(pic,ivec2(uv * size),0).xyz;
	outColor = vec4(sampleColor,1);

}

//这里简单直接使用纹理来作为材质，
void MaterialMapExample(){
	
	ivec2 size = textureSize(pic,0);
	vec3 sampleColor = texelFetch(materialPic,ivec2(inTexCoord.xy * size),0).xyz;
	//gama 解码，转线性空间
	sampleColor = pow(sampleColor, vec3(2.4));
	outColor = vec4(sampleColor,1);

}

void AlphaTextureExample(){
	//这里简单直接将alpha贴图贴上，不通过开启颜色混合和第二个pass的方式来绘制alpha贴图
	vec4 color = texture(alphaPic,inTexCoord.xy);
					//gama 解码，转线性空间
	color = pow(color, vec4(2.4));
	vec4 finalColor = vec4(1,1,1,1);//直接将平面绘制为白色
	if(color.a > 0.1)
	{
		//说明该贴图的颜色不透明，在这里手动进行颜色混合
		vec3 colorBack = finalColor.rgb;
		vec3 colorFront = color.rgb;
		vec3 tmpColor = color.a * colorFront + (1 - color.a) * colorBack;
		float tmpAlpha = color.a + finalColor.a * (1 - color.a);
		finalColor = vec4(tmpColor,tmpAlpha);


	
	}


	outColor = finalColor;


}


//凹凸贴图中的法线贴图   , 正确性时间不够，不再验证
void BumpMapping_NormalMapExample(){
	
	vec3 n = normalize(cross(inTan,inBiTan));
	
	mat3 TBN = mat3(inTan,inBiTan,n);
	vec3 sampleNormal = texture(normalPic,inTexCoord.xy).xyz;
	vec3 finalNormal = sampleNormal = normalize(TBN * sampleNormal);//纹理空间的法线转换到世界空间
	
	//定义一个定向光照射方向
	vec3 lightForward = vec3(0,3,-3);


	//计算phong光照


    // 光线方向
    vec3 lightDir = normalize(-lightForward);

    // 视线方向
    vec3 viewDir = normalize(cameraPos - inPosition);

    // 半程向量 (Halfway Vector)
    vec3 halfwayDir = normalize(lightDir + viewDir);

	vec3 lightColor = vec3(1);

    // 环境光
    vec3 ambient = vec3(0.3) * lightColor;

    // 漫反射 (Lambertian)
    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec3 diffuse = vec3(0.2) * diff * lightColor;

    // 镜面反射 (Blinn-Phong)
    float spec = pow(max(dot(finalNormal, halfwayDir), 0.0), 50);
    vec3 specular = vec3(1) * spec * lightColor;

    // 组合光照分量
    vec3 result = ambient + diffuse + specular;

    // 输出最终颜色
    outColor = vec4(result, 1.0);



}

//视差贴图中的视差遮挡贴图实例
void ParallaxOcclusionMappingExample(){


}


//纹理光源
void TextureLightExample(){


}

void main(){

//要显示的实例类型，0: 直接获取纹素值 ; 1:mipmap ;2: SAT; 3:随机生成的纹理;4:纹理动画;5:材质纹理
	if(exampleType == 0){
		RawFetch();
	}else if(exampleType == 1){
		MipMapExample();
	}else if(exampleType == 2)
	{
		SATExample();
	}else if(exampleType == 3)
	{
		ProcessTextureExample();
	}else if(exampleType == 4){
		TextureAnimateExample();
	}else if(exampleType == 5)
	{
		MaterialMapExample();
	}else if(exampleType == 6)
	{
		AlphaTextureExample();
	}else {
		outColor = vec4(1,0,0,1);
	}

}