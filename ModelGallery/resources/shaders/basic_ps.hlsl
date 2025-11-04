#include "Common.hlsli"
#include "Lighting.hlsli"

#define MAX_LIGHT 128

Texture2D AlbedoMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D SpecularMap : register(t2);
SamplerState TexSampler : register(s0);

cbuffer SceneConstants : register(b2)
{
	uint activeLightCount;
	float time;
	float deltaTime;
	uint shadowMapCount; // unused, jic for future implementation

	float3 cameraPosition;
	float _pad1;

	float2 screenResolution;
	float2 _pad0;

};

/*cbuffer MaterialData {

} material;*/

struct LightData
{
	float3	position;
	float3	direction;

	float	radiusSq;
	float3	color;

	uint	type;
	float2	coneAngles;
};

struct Input {
	float4 position : SV_POSITION;
	float3 worldPosition : POSITION;
	float2 uv : TEXCOORD0;
	float3x3 matTBN: NORMAL0;
};

struct Output {
	float4 color: SV_TARGET0;
};

StructuredBuffer<LightData> lightBuffer : register(t3);

[RootSignature( Default_RootSig )]
Output main(Input input) {
	float4 albedo = AlbedoMap.Sample(TexSampler, input.uv);
	if (albedo.a < 0.01f) clip(-1);

	float3 ambient = 0.05f;
	float3 color = albedo.rgb * ambient;
	float3 normal = 0.0f;
	float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
	float specularMask = SpecularMap.Sample(TexSampler, input.uv).r;
	float gloss = 128.0;
	float3 viewDir = normalize(cameraPosition - input.worldPosition);

	if (abs(dot(input.matTBN[0], input.matTBN[0])) > 0.000001) {
		normal = NormalMap.Sample(TexSampler, input.uv).rgb * 2.0 - 1.0;
		AntiAliasSpecular(normal, gloss);
		normal = normalize(mul(normal, input.matTBN));
	} else {
		normal = input.matTBN[2];
	}

	for (uint lightIndex = 0; lightIndex < activeLightCount; lightIndex++)
	{
		LightData light = lightBuffer[lightIndex];
		if (light.type == 2) {
			color += ApplyLightCommon(albedo.rgb, specularAlbedo, specularMask, gloss, normal, viewDir, -light.direction, light.color);
		}
	}


	Output output = (Output)0;
	float3 gammaCorrection = 1.0/2.2;
	output.color = float4(pow(color, gammaCorrection), albedo.a);
// 	normal = mad(normal, 0.5, 0.5);
// 	output.color = float4(normal, albedo.a);

	return output;
}
