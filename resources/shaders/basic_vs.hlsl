struct Input
{
    float4 position : POSITION;
    float4 color : COLOR;
};


struct Output
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


Output main(Input input)
{
    Output result;

    result.position = input.position;
    result.color = input.color;

    return result;
}
