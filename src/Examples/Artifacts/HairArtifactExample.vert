#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTangent;
layout(location = 3) out vec2 outTexCord;

layout(set = 0,binding = 0 ,std140) uniform TransBuffer{
	mat4 model;
	mat4 view;
	mat4 proj;
};



void main(){
  // 世界空间位置
    vec4 worldPosition = model * vec4(inPosition, 1.0);
    outPosition = worldPosition.xyz;

    // 法线和切线变换：用 model matrix 的前三列（忽略平移部分）
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    outNormal = normalize(normalMatrix * inNormal);
    outTangent = inTangent.w * normalize(normalMatrix * inTangent.xyz);

    // 纹理坐标直接传递
    outTexCord = inTexCord;

    // 最终顶点位置（NDC）
    gl_Position = proj * view * worldPosition;
}