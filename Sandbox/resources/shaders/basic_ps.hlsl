Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


struct Input {
	float4 position : SV_POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
};


float4 main(Input input) : SV_TARGET {
	float4 color = g_texture.Sample(g_sampler, input.uv);
	float4 lightDir = float4(0.2f, 0.3f, 0.8f, 1.0f);
	float4 dir = normalize(lightDir);
	float diff = saturate(dot(input.normal, dir));

	float3 col = pow(diff * color.rgb, 1.0 / 2.2); // gamma correction

	return float4(col, color.a);
}