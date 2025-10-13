#define MAX_LIGHT 128

Texture2D AlbedoMap;
Texture2D NormalMap;
Texture2D SpecularMap;
SamplerState TexSampler;

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

StructuredBuffer<LightData> lightBuffer;

float3 CalcDirectionalLight(LightData light, float3 normalMap, float3 diffMap, float3 viewDir, float specMask);

Output main(Input input) {
	float4 albedo = AlbedoMap.Sample(TexSampler, input.uv);
	if (albedo.a < 0.05f) clip(-1);

	float3 ambient = 0.05f;
	float3 color = albedo.rgb * ambient;
	float3 normal = 0.0f;
	float specularMask = SpecularMap.Sample(TexSampler, input.uv).r;

	float3 viewDir = normalize(cameraPosition - input.worldPosition);

	if (abs(dot(input.matTBN[0], input.matTBN[0])) > 0.000001) {
		normal = NormalMap.Sample(TexSampler, input.uv).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = normalize(mul(normal, input.matTBN));
	} else {
		normal = input.matTBN[2];
	}

	[unroll(MAX_LIGHT)]
	for (uint lightIndex = 0; lightIndex < activeLightCount; lightIndex++)
	{
		LightData light = lightBuffer[lightIndex];
		if (light.type == 2) {
			color += CalcDirectionalLight(light, normal, albedo.rgb, viewDir, specularMask);
		}
	}


	Output output = (Output)0;
	float3 gammaCorrection = 1.0/2.2;
	output.color = float4(pow(color, gammaCorrection), albedo.a);
// 	normal = mad(normal, 0.5, 0.5);
// 	output.color = float4(normal, albedo.a);

	return output;
}

float3 CalcDirectionalLight(LightData light, float3 N, float3 albedo, float3 V, float specMask)
{
	float3 L = normalize(-light.direction);
	float3 H = normalize(L + V);

	// Diffuse
	float NdotL = max(dot(N, L), 0.0);
	float3 diffuse = light.color * NdotL;

	// Specular
	float NdotH = max(dot(N, H), 0.0);
	float specular = pow(NdotH, 64.0f) * 0.5f * specMask; // 32 : shininess, 0.5 : specular strength
	float3 specularColor = light.color * specular * step(0.0, NdotL);

	// Ambient
	float3 ambient = light.color * 0.1f; // 0.1: ambient strength

	// Combine
	float3 finalColor = albedo * (ambient + diffuse) + specularColor;
	return finalColor;
}
