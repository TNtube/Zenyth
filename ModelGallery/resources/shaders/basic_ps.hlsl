Texture2D AlbedoMap;
SamplerState AlbedoSampler;
Texture2D NormalMap;
SamplerState NormalSampler;

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

float3 CalcDirectionalLight(LightData light, float3 normalMap, float3 diffMap, float3 viewDir);

Output main(Input input) {
	float4 albedo = AlbedoMap.Sample(AlbedoSampler, input.uv);

	float3 ambient = 0.05f;
	float3 color = albedo.rgb * ambient;
	float3 normal = 0.0f;

	float3 viewDir = normalize(cameraPosition - input.worldPosition);

	if (abs(dot(input.matTBN[0], input.matTBN[0])) > 0.000001) {
		normal = NormalMap.Sample(NormalSampler, input.uv).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = normalize(mul(input.matTBN, normal));
	} else {
		normal = input.matTBN[2];
	}

	for (uint lightIndex = 0; lightIndex < 1; lightIndex++)
	{
		LightData light = lightBuffer[lightIndex];
		if (light.type == 2) {
			color += CalcDirectionalLight(light, normal, albedo.rgb, viewDir);
		}
	}


	Output output = (Output)0;
	float3 gammaCorrection = 1.0/2.2;
	output.color = float4(pow(color, gammaCorrection), albedo.a);

	return output;
}

float3 CalcDirectionalLight(LightData light, float3 normalMap, float3 diffMap, float3 viewDir)
{
	float3 lightDir = normalize(-light.direction);

	// diffuse shading
	float diff = max(dot(normalMap, lightDir), 0.0);
	// specular shading
	float3 reflectDir = reflect(-lightDir, normalMap);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0f);
	// combine results
	float3 diffuse  = light.color  * diff * diffMap;
	float3 specular = spec;
	return (diffuse);
}