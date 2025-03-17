Texture2D<float4> MainTexture : register(t0);

float4 main(float4 position : SV_POSITION) : SV_TARGET {
	return MainTexture[(int2)position.xy];
}