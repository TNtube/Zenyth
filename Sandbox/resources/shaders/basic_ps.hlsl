
void AntiAliasSpecular( inout float3 texNormal, inout float gloss )
{
	float normalLenSq = dot(texNormal, texNormal);
	float invNormalLen = rsqrt(normalLenSq);
	texNormal *= invNormalLen;
	float normalLen = normalLenSq * invNormalLen;
	float flatness = saturate(1 - abs(ddx(normalLen)) - abs(ddy(normalLen)));
	gloss = exp2(lerp(0, log2(gloss), flatness));
}

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


Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


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


float4 main(Input input) : SV_TARGET {
	float3 lightDir = normalize(float3(sin(input.time), -1, -0.5));
	// ambient light
	float3 ambient = 0.0005;

	float3 lightPos = float3(2, 72, 0);

# define SAMPLE_TEX(texName) texName.Sample(g_sampler, input.uv)

	float4 diffuseAlbedo = SAMPLE_TEX(g_texture);
	clip(diffuseAlbedo.a < 0.1 ? -1 : 1);

	float3 colorSum = ambient * diffuseAlbedo.rgb;
	float gloss = 128.0;
	float4 normal = normalize(input.normal);
	// {
	// 	AntiAliasSpecular(normal.rgb, gloss);
	// 	normal = normalize(normal);
	// }

	float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
	colorSum += ApplyLightCommon(diffuseAlbedo.rgb, specularAlbedo, normal.rgb, input.viewDir, input.worldPosition.xyz, input.camPos, 1000, float3(0.4, 0.4f, 0.4), input.camDir, float2(0.8, 0.8));

	float3 col = pow(colorSum, 1.0 / 2.2); // gamma correction

	return float4(col, diffuseAlbedo.a);
}