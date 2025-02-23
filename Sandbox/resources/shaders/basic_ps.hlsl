Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


struct Input {
	float4 position : SV_POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
};


float4 main(Input input) : SV_TARGET {
	float4 color = g_texture.Sample(g_sampler, input.uv);

	clip(color.a < 0.1 ? -1 : 1);

	float NdotL = saturate(dot(input.normal, float4(1, 1, 1, 0)));
	color.rgb = color.rgb * (0.5 + NdotL * 0.5);

	float3 col = pow(color.rgb, 1.0 / 2.2); // gamma correction

	return float4(col, color.a);
}