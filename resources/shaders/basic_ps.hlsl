Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


struct Input
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};


float4 main(Input input) : SV_TARGET {
	float4 color = g_texture.Sample(g_sampler, input.uv);
	return color;
}