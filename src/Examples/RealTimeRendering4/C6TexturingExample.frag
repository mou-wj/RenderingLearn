#version 450 core

//�������⣺
//SAT��������ȫ0������
//������ͼ��ȷ����֤
//�Ӳ���ͼ
//�����Դ

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��е�λ��
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;

layout(set = 0,binding = 1) uniform sampler2D pic;
// �����󶨵�ͼ��
layout(set = 0,binding = 2, rgba32f) readonly uniform image2D myImage;


layout(set = 0,binding = 3,std140) uniform Info{
	float animateDelta;
	layout(offset = 16) vec3 cameraPos;//����ռ������λ��
	layout(offset = 32) int exampleType;//Ҫ��ʾ��ʵ�����ͣ�0: ֱ�ӻ�ȡ����ֵ ; 1:mipmap ;2: SAT; 3:������ɵ�����;4:������;5:��������;6:alpha��ͼ
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
	float depth = gl_FragCoord.z; // ��ǰƬ�ε����ֵ
	float deltaD = 1 / 8;//�������8��mipmap�ȼ�

	uint curMipLevel = uint(depth / deltaD);

	vec3 sampleColor = textureLod(pic,inTexCoord.xy,curMipLevel).xyz;
	outColor = vec4(sampleColor,1);

}

void SATExample(){
	
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
void ProcessTextureExample(){
	vec3 color = ProcessTexture(inTexCoord.xy);
	outColor = vec4(color,1);

}

//��������ͨ���ı�ÿһ֡��uvֵ��ʵ��������
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

//�����ֱ��ʹ����������Ϊ���ʣ�
void MaterialMapExample(){
	
	ivec2 size = textureSize(pic,0);
	vec3 sampleColor = texelFetch(materialPic,ivec2(inTexCoord.xy * size),0).xyz;
	//gama ���룬ת���Կռ�
	sampleColor = pow(sampleColor, vec3(2.4));
	outColor = vec4(sampleColor,1);

}

void AlphaTextureExample(){
	//�����ֱ�ӽ�alpha��ͼ���ϣ���ͨ��������ɫ��Ϻ͵ڶ���pass�ķ�ʽ������alpha��ͼ
	vec4 color = texture(alphaPic,inTexCoord.xy);
					//gama ���룬ת���Կռ�
	color = pow(color, vec4(2.4));
	vec4 finalColor = vec4(1,1,1,1);//ֱ�ӽ�ƽ�����Ϊ��ɫ
	if(color.a > 0.1)
	{
		//˵������ͼ����ɫ��͸�����������ֶ�������ɫ���
		vec3 colorBack = finalColor.rgb;
		vec3 colorFront = color.rgb;
		vec3 tmpColor = color.a * colorFront + (1 - color.a) * colorBack;
		float tmpAlpha = color.a + finalColor.a * (1 - color.a);
		finalColor = vec4(tmpColor,tmpAlpha);


	
	}


	outColor = finalColor;


}


//��͹��ͼ�еķ�����ͼ   , ��ȷ��ʱ�䲻����������֤
void BumpMapping_NormalMapExample(){
	
	vec3 n = normalize(cross(inTan,inBiTan));
	
	mat3 TBN = mat3(inTan,inBiTan,n);
	vec3 sampleNormal = texture(normalPic,inTexCoord.xy).xyz;
	vec3 finalNormal = sampleNormal = normalize(TBN * sampleNormal);//����ռ�ķ���ת��������ռ�
	
	//����һ����������䷽��
	vec3 lightForward = vec3(0,3,-3);


	//����phong����


    // ���߷���
    vec3 lightDir = normalize(-lightForward);

    // ���߷���
    vec3 viewDir = normalize(cameraPos - inPosition);

    // ������� (Halfway Vector)
    vec3 halfwayDir = normalize(lightDir + viewDir);

	vec3 lightColor = vec3(1);

    // ������
    vec3 ambient = vec3(0.3) * lightColor;

    // ������ (Lambertian)
    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec3 diffuse = vec3(0.2) * diff * lightColor;

    // ���淴�� (Blinn-Phong)
    float spec = pow(max(dot(finalNormal, halfwayDir), 0.0), 50);
    vec3 specular = vec3(1) * spec * lightColor;

    // ��Ϲ��շ���
    vec3 result = ambient + diffuse + specular;

    // ���������ɫ
    outColor = vec4(result, 1.0);



}

//�Ӳ���ͼ�е��Ӳ��ڵ���ͼʵ��
void ParallaxOcclusionMappingExample(){


}


//�����Դ
void TextureLightExample(){


}

void main(){

//Ҫ��ʾ��ʵ�����ͣ�0: ֱ�ӻ�ȡ����ֵ ; 1:mipmap ;2: SAT; 3:������ɵ�����;4:������;5:��������
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