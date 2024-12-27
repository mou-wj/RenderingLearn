#version 450

// ���룺�Ӷ�����ɫ�������Ķ�������
layout(triangles) in;  // ��������Ϊ������

// ��������ݸ�Ƭ����ɫ���Ķ�������
layout(line_strip,max_vertices  = 3) out;  // �������Ϊ�����δ���triangle strip�������6������



void main() {
    // ������������������Σ�ʹ����һ��������
    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;  // ����ԭʼ����λ��
        EmitVertex();  // ����һ������

        gl_Position = gl_in[(i + 1) % 3].gl_Position;  // �ƶ�����һ������
        EmitVertex();  // ����һ������

        gl_Position = gl_in[(i + 2) % 3].gl_Position;  // �ƶ�������������
        EmitVertex();  // ����һ������

        EndPrimitive();  // �������ͼԪ
    }
}
