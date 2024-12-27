#version 450

// 输入：从顶点着色器传来的顶点数据
layout(triangles) in;  // 输入类型为三角形

// 输出：传递给片段着色器的顶点数据
layout(line_strip,max_vertices  = 3) out;  // 输出类型为三角形带（triangle strip），最多6个顶点



void main() {
    // 生成两个额外的三角形，使其变成一个六边形
    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;  // 传递原始顶点位置
        EmitVertex();  // 发射一个顶点

        gl_Position = gl_in[(i + 1) % 3].gl_Position;  // 移动到下一个顶点
        EmitVertex();  // 发射一个顶点

        gl_Position = gl_in[(i + 2) % 3].gl_Position;  // 移动到第三个顶点
        EmitVertex();  // 发射一个顶点

        EndPrimitive();  // 结束这个图元
    }
}
