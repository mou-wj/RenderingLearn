#version 450


// ����ÿ�������Ŀ��Ƶ�����
layout(vertices = 3) out; // 3 ���������β��� (triangular patches)

// ���룺���Զ�����ɫ���Ŀ��Ƶ�����
layout(location = 0) in vec3 inPosition[];

// ��������ݸ�ϸ��������ɫ��������
layout(location = 0) out vec3 outPosition[];
layout(location = 1) out vec3 outColor[];

// ���Ƶ�������ÿ�����Ƶ㶼��ִ��һ�Σ�
void main() {
    // ���������ݴ��ݸ��������
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];

    // ֻ��Ҫ��һ�����Ƶ㸺������ϸ�ּ���ͨ��ѡ�� ID=0��
    if (gl_InvocationID == 0) {
        // �ⲿϸ�����ӣ������������ε�������
        gl_TessLevelOuter[0] = 4.0; // ��1
        gl_TessLevelOuter[1] = 4.0; // ��2
        gl_TessLevelOuter[2] = 4.0; // ��3

        // �ڲ�ϸ�����ӣ�������Ϊ1��ֵ���ı���Ϊ2��ֵ
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