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

    result.position = input.position;
    result.uv = input.uv;

    return result;
}
