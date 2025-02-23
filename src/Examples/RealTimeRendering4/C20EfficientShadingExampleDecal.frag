#version 450 core


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 0) out vec4 outColor1;
layout(rgba32f,set = 0,binding = 2) uniform image2D GBuffer1;//存放有深度信息
layout(set = 0,binding = 3) uniform Transform{
	mat4 mainTransform;

};//

layout(set = 0,binding = 4) uniform sampler2D decalTexture;//贴花的纹理

void main(){
	
	//将当前位置变换到实际主界面的位置，看是否可见
	vec4 tmp1 = mainTransform * vec4(inPosition,1.0);
	tmp1 /=tmp1.w;

	ivec2 size = imageSize(GBuffer1);
	vec2 uv = (tmp1.xy + 1) / 2;

	ivec2 fcoord = ivec2( uv * size);

	float depth = imageLoad(GBuffer1,fcoord).w;

	//说明不可见
	if(tmp1.z - 0.00001 > depth)
	{
		discard	;
	}
	
	//如果可见，将Gbuffer中的颜色设置为decal的颜色
	vec3 decalColor = texture(decalTexture,inUV).xyz;
	
	imageStore(GBuffer1,fcoord,vec4(decalColor,depth));


	outColor1 = vec4(decalColor,1);
	
}