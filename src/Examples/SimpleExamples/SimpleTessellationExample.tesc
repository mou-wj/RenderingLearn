#version 450


// 定义每个补丁的控制点数量
layout(vertices = 3) out; // 3 代表三角形补丁 (triangular patches)

// 输入：来自顶点着色器的控制点数据
layout(location = 0) in vec3 inPosition[];

// 输出：传递给细分评估着色器的数据
layout(location = 0) out vec3 outPosition[];
layout(location = 1) out vec3 outColor[];

// 控制点索引（每个控制点都会执行一次）
void main() {
    // 将输入数据传递给输出数组
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];

    // 只需要由一个控制点负责设置细分级别（通常选择 ID=0）
    if (gl_InvocationID == 0) {
        // 外部细分因子：适用于三角形的三个边
        gl_TessLevelOuter[0] = 4.0; // 边1
        gl_TessLevelOuter[1] = 4.0; // 边2
        gl_TessLevelOuter[2] = 4.0; // 边3

        // 内部细分因子：三角形为1个值，四边形为2个值
        gl_TessLevelInner[0] = 3;
    }

    if (gl_InvocationID == 0) {
        outColor[gl_InvocationID]  =vec3(1,0,0);
    }else if(gl_InvocationID == 1){
        outColor[gl_InvocationID]  =vec3(0,1,0);
    }else if(gl_InvocationID == 2){
        outColor[gl_InvocationID]  =vec3(0,0,1);
    }else {
        outColor[gl_InvocationID]  =vec3(0,0,0);
    }

}