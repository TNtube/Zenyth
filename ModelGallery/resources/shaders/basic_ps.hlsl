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

struct LightData
{
	float3	position;
	float3	direction;

	float	radiusSq;
	float3	color;

	uint	type;
	float2	coneAngles;
};

StructuredBuffer<LightData> lightBuffer;

Output main(Input input) {
	// ambient light

	float3 N = input.matTBN[2];

	float3 lightColor  = lightBuffer[0].color;
	float3 lightDir    = normalize(lightBuffer[0].direction);
	float  diff        = max(dot(N, lightDir), 0.0);

	float3 diffuse = lightColor * diff;
	float3 ambient = lightColor * 0.1;


	float4 diffuseAlbedo = AlbedoTexture.Sample(AlbedoSampler, input.uv);
	float4 normalMap = NormalTexture.Sample(NormalSampler, input.uv);

	float3 color = (ambient + diffuse) * diffuseAlbedo.rgb;


	Output output = (Output)0;
	float3 gammaCorrection = 1.0/2.2;
	output.color = float4(pow(color, gammaCorrection), diffuseAlbedo.a);

	return output;
}