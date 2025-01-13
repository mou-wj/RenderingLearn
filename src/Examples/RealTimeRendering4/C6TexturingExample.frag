#version 450 core


layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的位置
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 1) uniform sampler2D pic;
// 声明绑定的图像
layout(set = 0,binding = 2, rgba32f) readonly uniform image2D myImage;


layout(set = 0,binding = 3,std140) uniform Info{
	float animateDelta;
};

void RawFetch(){
	ivec2 size = textureSize(pic,0);
	vec3 sampleColor = texelFetch(pic,ivec2(inTexCoord.xy * size),0).xyz;
	outColor = vec4(sampleColor,1);

}

void MipMapSample(){
	float depth = gl_FragCoord.z; // 当前片段的深度值
	float deltaD = 1 / 8;//假设最大8个mipmap等级

	uint curMipLevel = uint(depth / deltaD);

	vec3 sampleColor = textureLod(pic,inTexCoord.xy,curMipLevel).xyz;
	outColor = vec4(sampleColor,1);

}

void SATSample(){
	
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
void ProcessTextureSample(){
	vec3 color = ProcessTexture(inTexCoord.xy);
	outColor = vec4(color,1);

}

//纹理动画，通过改变每一帧的uv值来实现纹理动画
void TextureAnimateSample(){
	vec2 uv = inTexCoord.xy;
	uv.y += animateDelta;
	if(uv.y > 1){
		uv.y -= 1;
	
	}

	ivec2 size = textureSize(pic,0);
	vec3 sampleColor = texelFetch(pic,ivec2(uv * size),0).xyz;
	outColor = vec4(sampleColor,1);

}


void MaterialMapSample(){
	
	

}

void main(){

	ProcessTextureSample();

}