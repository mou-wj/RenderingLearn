#version 450 core
layout(set = 0,binding = 3,std140) uniform ShadeBuffer{
	vec4 baseColorFactor;
	vec2 metalicAndRouguness; 
    vec3 pointLightPos;
    vec3 pointLightColor;
    vec3 viewPos;
};

layout(set = 0,binding = 1) uniform sampler2D baseColorTexture;
layout(set = 0,binding = 2) uniform sampler2D normalTexture;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

// 从法线贴图中获取扰动后的法线（切线空间 -> 世界空间）
vec3 getPerturbedNormal()
{
    vec3 N = normalize(inNormal);
    vec3 T = normalize(inTangent);
    T = normalize(T - dot(T, N) * N); // 正交化
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    vec3 normalTex = texture(normalTexture, inTexCoord).xyz;
    normalTex = normalTex * 2.0 - 1.0; // 转换为 [-1, 1] 空间
    return normalize(TBN * normalTex); // 转换到世界空间
}

void main()
{
    vec4 baseColor = texture(baseColorTexture, inTexCoord).rgba * baseColorFactor.rgba;

    vec3 normal = getPerturbedNormal();
    vec3 fragPos = inPosition;
    
    vec3 lightDir = normalize(pointLightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    // 漫反射 (Lambert)
    float diff = max(dot(normal, lightDir), 0.0);

    // 高光 (Blinn-Phong)
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0); // 32 为高光强度

    vec3 ambient = 0.1 * baseColor.rgb;
    vec3 diffuse = diff * baseColor.rgb * pointLightColor;
    vec3 specular = spec * vec3(1.0) * pointLightColor;

    vec3 result = ambient + diffuse + specular;

    outColor = vec4(result, baseColorFactor.a);
    outColor = vec4(baseColor);
}