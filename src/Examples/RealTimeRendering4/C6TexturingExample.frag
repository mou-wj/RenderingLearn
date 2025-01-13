#version 450 core


layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��е�λ��
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 1) uniform sampler2D pic;
// �����󶨵�ͼ��
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
	float depth = gl_FragCoord.z; // ��ǰƬ�ε����ֵ
	float deltaD = 1 / 8;//�������8��mipmap�ȼ�

	uint curMipLevel = uint(depth / deltaD);

	vec3 sampleColor = textureLod(pic,inTexCoord.xy,curMipLevel).xyz;
	outColor = vec4(sampleColor,1);

}

void SATSample(){
	
	ivec2 size = textureSize(pic,0);
	
	    // ���㵱ǰƬ�ε���������ƫ����
	vec2 fragTexCoord = inTexCoord.xy;
    vec2 dx = dFdx(fragTexCoord);
    vec2 dy = dFdy(fragTexCoord);

    // ���㵱ǰƬ���������еĸ��Ƿ�Χ
    vec2 uvMin = fragTexCoord - 0.5 * abs(dx) - 0.5 * abs(dy);
    vec2 uvMax = fragTexCoord + 0.5 * abs(dx) + 0.5 * abs(dy);

	// �� uvMin �� uvMax ת��Ϊ����ռ����������
    ivec2 texMin = ivec2(uvMin * size);
    ivec2 texMax = ivec2(uvMax * size);

	int deltaU = texMax.x-texMin.x;
	int deltaV = texMax.y-texMin.y;
	int numTexels = deltaU * deltaV;

	// ���ϡ����ϡ����¡������ĸ���������
    ivec2 topLeft = texMin;
    ivec2 topRight = ivec2(texMax.x, texMin.y);
    ivec2 bottomLeft = ivec2(texMin.x, texMax.y);
    ivec2 bottomRight = texMax;


	//�����ȡ��ֵ����Ϊ0������������ȷ�������Ϊ0������������Ǹ�ʽ������,Ҳ�п��������image���ݵ�ʱ��������⣬����ʱ����
	//vec3 sumColorTopLeft = texelFetch(pic, topLeft, 0).xyz;
    //vec3 sumColorTopRight = texelFetch(pic, topRight, 0).xyz;
    //vec3 sumColorBottomLeft = texelFetch(pic, bottomLeft, 0).xyz;
    //vec3 sumColorBottomRight = texelFetch(pic, bottomRight, 0).xyz;
	vec3 sumColorTopLeft = imageLoad(myImage, topLeft).xyz;
    vec3 sumColorTopRight = imageLoad(myImage, topRight).xyz;
    vec3 sumColorBottomLeft = imageLoad(myImage, bottomLeft).xyz;
    vec3 sumColorBottomRight = imageLoad(myImage, bottomRight).xyz;

    // �����ܵ� texel ����
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

//ֱ��ͨ������õ�������ֵ
void ProcessTextureSample(){
	vec3 color = ProcessTexture(inTexCoord.xy);
	outColor = vec4(color,1);

}

//��������ͨ���ı�ÿһ֡��uvֵ��ʵ��������
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