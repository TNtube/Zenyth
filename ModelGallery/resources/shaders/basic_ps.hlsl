Texture2D AlbedoTexture;
SamplerState AlbedoSampler;
Texture2D NormalTexture;
SamplerState NormalSampler;

struct Input {
	float4 position : SV_POSITION;
	float3 worldPosition : POSITION;
	float2 uv : TEXCOORD0;
	float3x3 matTBN: NORMAL0;
	float time : TIME;
};

struct Output {
	float4 color: SV_TARGET0;
};

Output main(Input input) {
	// ambient light
	float3 ambient = 0.1;


	float4 diffuseAlbedo = AlbedoTexture.Sample(AlbedoSampler, input.uv);
	float4 normalMap = NormalTexture.Sample(NormalSampler, input.uv);


	Output output = (Output)0;
	float gammaCorrection = 1.0/2.2;
	output.color = float4(pow(diffuseAlbedo.rgb, float3(gammaCorrection, gammaCorrection, gammaCorrection)), diffuseAlbedo.a);

	return output;
}