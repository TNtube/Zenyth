Texture2D<float4> MainTexture : register(t0);

float4 main(float4 position : SV_POSITION) : SV_TARGET {

	float4 color = MainTexture[(int2)position.xy];

	// float4 corrected = pow(color, 1.0 / 2.2);

	return color;
}