cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
    float4 padding[15];
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
    Output result;

    result.position = input.position + offset;
    result.uv = input.uv;

    return result;
}
