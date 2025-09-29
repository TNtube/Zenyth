void FSchlick( inout float3 specular, inout float3 diffuse, float3 lightDir, float3 halfVec )
{
	float fresnel = pow(1.0 - saturate(dot(lightDir, halfVec)), 5.0);
	specular = lerp(specular, 1, fresnel);
	diffuse = lerp(diffuse, 0, fresnel);
}


float3 ApplyLightCommon(
	float3	diffuseColor,	// Diffuse albedo
	float3	specularColor,	// Specular albedo
	float3	normal,			// World-space normal
	float3	viewDir,		// World-space vector from eye to point
	float3	worldPos,		// World-space fragment position
	float3	lightPos,		// World-space light position
	float	lightRadiusSq,
	float3	lightColor,		// Radiance of directional light
	float3	coneDir,
	float2	coneAngles
	)
{

	// point light

	float3 lightDir = lightPos - worldPos;
	float lightDistSq = dot(lightDir, lightDir);
	float invLightDist = rsqrt(lightDistSq);
	lightDir *= invLightDist;

	float distanceFalloff = lightRadiusSq * (invLightDist * invLightDist);
	distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

	// spot light
	float coneFalloff = dot(-lightDir, coneDir);
	coneFalloff = saturate((coneFalloff - coneAngles.y) * coneAngles.x);


	// common light calculations
	float3 halfVec = normalize(lightDir - viewDir);
	float nDotH = saturate(dot(halfVec, normal));

	FSchlick( diffuseColor, specularColor, lightDir, halfVec );

	float nDotL = saturate(dot(normal, lightDir));

	// mix
	return coneFalloff * distanceFalloff * nDotL * lightColor * diffuseColor;
}


Texture2D AlbedoTexture;
SamplerState AlbedoSampler;
Texture2D NormalTexture;
SamplerState NormalSampler;


struct Input {
	float4 position : SV_POSITION;
	float4 worldPosition : POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
	float3 viewDir: TEXCOORD1;
	float3 camPos: TEXCOORD2;
	float3 camDir: TEXCOORD3;
	float time : TIME;
};

struct Output {
	float4 color: SV_TARGET0;
};

Output main(Input input) {
	// ambient light
	float3 ambient = 0.1;

	float4 diffuseAlbedo = AlbedoTexture.Sample(AlbedoSampler, input.uv);
	float4 normal = NormalTexture.Sample(NormalSampler, input.uv);

	normal = normal-normal + input.normal;

	//clip(diffuseAlbedo.a < 0.1 ? -1 : 1);

	float3 colorSum = ambient * diffuseAlbedo.rgb;

	float3 specularAlbedo = float3( 0.0, 0.0, 0.0);
	colorSum += ApplyLightCommon(diffuseAlbedo.rgb, specularAlbedo, normal.rgb, input.viewDir, input.worldPosition.xyz, input.camPos, 1000, float3(0.01, 0.01, 0.01), input.camDir, float2(0.8, 0.8));

	float3 col = pow(colorSum, 1.0 / 2.2);

	Output output = (Output)0;

	output.color = float4(col, 1.0);

	return output;
}