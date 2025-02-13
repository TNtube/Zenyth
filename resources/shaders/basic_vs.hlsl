cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 model;
    float4 padding[12];
};

cbuffer CameraConstantBuffer : register(b1)
{
    float4x4 view;
    float4x4 projection;
    // padding
    float4x4 cameraPadding[2];
};


struct Input
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};


struct Output
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};


Output main(Input input)
{
	Output output = (Output)0;
	output.position = mul( input.position, model );
//    output.position = input.position + offset;
	output.position = mul( output.position, view );
	output.position = mul( output.position, projection );
	output.uv = input.uv;

    return output;
}
