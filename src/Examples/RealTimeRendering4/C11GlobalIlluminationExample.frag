#version 450 core

layout(location = 0) in vec3 inPosition;//����ռ��е�λ��
layout(location = 1) in vec3 inNormal;//����ռ��еķ���
layout(location = 2) in vec3 inTexCoord;//����ռ��е�λ��
layout(location = 3) in vec3 inTan;//����ռ��е�Tan����
layout(location = 4) in vec3 inBiTan;//����ռ��е�BiTan����
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

layout(set = 0,binding = 2) uniform sampler2D depthTexture;
layout(set = 0,binding = 1) uniform Info{
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    uint exampleType;//ʾ���ࡣ0��ʾSSAO��1��ʾVOAO



};
//���������
// ����Hammersley ���е�һά�����㣬����ά�ȵ����п���ʹ�ò�ͬ�Ļ�������
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
// ���ɾ��ȷֲ��Ķ�άHalton������
vec2 HaltonSample2D(uint index) {
    return vec2(HammersleySequence(index, 2),HammersleySequence(index, 3)); // ����ѡ�� 2,3
}


// ���ɾ��ȷֲ�����άHalton������
vec3 HaltonSample3D(uint index) {
    return vec3(HammersleySequence(index, 2),HammersleySequence(index, 3),HammersleySequence(index, 5)); // ����ѡ�� 2,3,5
}


//�����ռ�Ļ������ڱ�SSAO
void ScreenSpaceAmbientOcclusionExample(){
	
    uint numSample = 40;
    float ka = 0;

    for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++){
        vec3 samplePoint = HaltonSample3D(sampleIndex);
        vec3 targetV;
        float phi = 2 * s_pi * samplePoint.x;
        float theta = 0.5 * s_pi * (samplePoint.y - 0.5);
        targetV.y = -cos(theta);
        targetV.x = sin(theta) * cos(phi);
        targetV.z = sin(theta) * sin(phi);
        float d = samplePoint.z * 0.5 + 0.0001;

        vec3 realSamplePointWorldPos = inPosition + targetV * d;

        //��������������������е����ֵ
        vec4 samplePosInScreen = proj * view * vec4(realSamplePointWorldPos,1.0);
        samplePosInScreen/=samplePosInScreen.w;



        //��ȡ�����������ͼ�ж�Ӧ�����ֵ
        float depthInScreen = texture(depthTexture,(samplePosInScreen.xy+1)/2).x;

        //��������ͨ����Ȳ���ka+1
        if(depthInScreen >= samplePosInScreen.z)
        {
            ka+=1 * (1 - samplePoint.z);//��������뵱ǰλ��ԽС����ȨֵԽ��        
        }    
    
    }

    ka/=numSample;

    outColor = vec4(ka,ka,ka,1);

}


void VolumetricObscuranceAmbientOcclusionExample(){

    uint numSample = 40;

    float halfRange = 0.1;//���������Χ��һ��
    float sampleSphereRadius = 0.2;//���������������Χ�İ뾶

    float ka = 0;

    mat4 inverseView = inverse(view);
    mat4 inverseProj = inverse(proj);
    vec3 v = normalize(cameraPos - inPosition);

     vec4 curPosInScreen = proj * view * vec4(inPosition,1.0);
    curPosInScreen/=curPosInScreen.w;
    float curDepthInMap = texture(depthTexture,(curPosInScreen.xy+1) / 2).x;

    for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++){
        vec2 samplePoint = HaltonSample2D(sampleIndex);
        //���㵱ǰλ������������е�uv����
       
       


        //�������ͼ�еĵ�
        vec2 samplePointXYInDepthMap =curPosInScreen.xy + (samplePoint.xy - 0.5)*2 *halfRange;
        vec2 sampleUV = (samplePointXYInDepthMap.xy+1) / 2;;
        float samplePointDepthInDepthMap = texture(depthTexture,sampleUV).x;
        vec4 samplePointInDepthMap = vec4(samplePointXYInDepthMap,samplePointDepthInDepthMap,1.0);


        //��任�鿴�������Ƿ��ڵ�ǰ��Ĳ�������ϰ���Χ��
        vec4 samplePointRealPosInWorld = inverseView * inverseProj * samplePointInDepthMap;
        samplePointRealPosInWorld/=samplePointRealPosInWorld.w;

        vec3 s =normalize( samplePointRealPosInWorld.xyz - inPosition);
        float d = distance(samplePointRealPosInWorld.xyz,inPosition);


        //����ڲ�������ķ�Χ��
        if(d <= sampleSphereRadius){
            //����ռ�ò��ֺ�δռ�ò��ֵı���
            float ratio = 0;
            float sDotV = dot(v,s);
            float sinSV = abs(sqrt(1 - pow(sDotV,2)));

            //��������
            float perpLen = sqrt(pow(sampleSphereRadius,2) - pow(d * sinSV,2));
            float dHide = 0;
            if(sDotV>=0)
            {
                dHide = perpLen + d * abs(sDotV);
            }else {
                dHide = perpLen - d * abs(sDotV);
            }
            
            float dUp = 2 * perpLen - dHide;
            ratio = dUp / dHide;
            ka+=min(ratio,1.0);
        
        }
   }  


    ka/=numSample;

    outColor = vec4(ka,ka,ka,1);

    


}

//to do
//���㻷����ͼ����гϵ����Ȼ�������гϵ������Ļ�ռ��������ȫ�ֹ����Լ�����ȫ�ֹ���

void main(){
    if(exampleType == 0)
    {
        ScreenSpaceAmbientOcclusionExample();
    }else if(exampleType == 1)
    {
        VolumetricObscuranceAmbientOcclusionExample();
    }


}