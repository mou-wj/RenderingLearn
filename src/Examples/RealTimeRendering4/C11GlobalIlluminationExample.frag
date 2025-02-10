#version 450 core

layout(location = 0) in vec3 inPosition;//世界空间中的位置
layout(location = 1) in vec3 inNormal;//世界空间中的法线
layout(location = 2) in vec3 inTexCoord;//世界空间中的位置
layout(location = 3) in vec3 inTan;//世界空间中的Tan向量
layout(location = 4) in vec3 inBiTan;//世界空间中的BiTan向量
layout(location = 0) out vec4 outColor;

const float s_pi = 3.141592653;

layout(set = 0,binding = 2) uniform sampler2D depthTexture;
layout(set = 0,binding = 1) uniform Info{
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    uint exampleType;//示例类。0表示SSAO，1表示VOAO



};
//生成随机数
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


// 生成均匀分布的三维Halton采样点
vec3 HaltonSample3D(uint index) {
    return vec3(HammersleySequence(index, 2),HammersleySequence(index, 3),HammersleySequence(index, 5)); // 基数选择 2,3,5
}


//环境空间的环境光遮蔽SSAO
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

        //计算采样点在纹理坐标中的深度值
        vec4 samplePosInScreen = proj * view * vec4(realSamplePointWorldPos,1.0);
        samplePosInScreen/=samplePosInScreen.w;



        //获取采样点在深度图中对应的深度值
        float depthInScreen = texture(depthTexture,(samplePosInScreen.xy+1)/2).x;

        //当采样点通过深度测试ka+1
        if(depthInScreen >= samplePosInScreen.z)
        {
            ka+=1 * (1 - samplePoint.z);//如果距离离当前位置越小，其权值越高        
        }    
    
    }

    ka/=numSample;

    outColor = vec4(ka,ka,ka,1);

}


void VolumetricObscuranceAmbientOcclusionExample(){

    uint numSample = 40;

    float halfRange = 0.1;//定义采样范围的一半
    float sampleSphereRadius = 0.2;//定义采样点所在球范围的半径

    float ka = 0;

    mat4 inverseView = inverse(view);
    mat4 inverseProj = inverse(proj);
    vec3 v = normalize(cameraPos - inPosition);

     vec4 curPosInScreen = proj * view * vec4(inPosition,1.0);
    curPosInScreen/=curPosInScreen.w;
    float curDepthInMap = texture(depthTexture,(curPosInScreen.xy+1) / 2).x;

    for(uint sampleIndex = 0;sampleIndex < numSample;sampleIndex++){
        vec2 samplePoint = HaltonSample2D(sampleIndex);
        //计算当前位置在深度纹理中的uv坐标
       
       


        //采样深度图中的点
        vec2 samplePointXYInDepthMap =curPosInScreen.xy + (samplePoint.xy - 0.5)*2 *halfRange;
        vec2 sampleUV = (samplePointXYInDepthMap.xy+1) / 2;;
        float samplePointDepthInDepthMap = texture(depthTexture,sampleUV).x;
        vec4 samplePointInDepthMap = vec4(samplePointXYInDepthMap,samplePointDepthInDepthMap,1.0);


        //逆变换查看采样点是否在当前点的采样球的上半球范围内
        vec4 samplePointRealPosInWorld = inverseView * inverseProj * samplePointInDepthMap;
        samplePointRealPosInWorld/=samplePointRealPosInWorld.w;

        vec3 s =normalize( samplePointRealPosInWorld.xyz - inPosition);
        float d = distance(samplePointRealPosInWorld.xyz,inPosition);


        //如果在采样半球的范围内
        if(d <= sampleSphereRadius){
            //计算占用部分和未占用部分的比例
            float ratio = 0;
            float sDotV = dot(v,s);
            float sinSV = abs(sqrt(1 - pow(sDotV,2)));

            //垂径长度
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
//计算环境贴图的球谐系数，然后基于球谐系数的屏幕空间的漫反射全局光照以及镜面全局光照

void main(){
    if(exampleType == 0)
    {
        ScreenSpaceAmbientOcclusionExample();
    }else if(exampleType == 1)
    {
        VolumetricObscuranceAmbientOcclusionExample();
    }


}