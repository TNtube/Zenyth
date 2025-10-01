cbuffer ModelData {
	float4x4 model;
};

cbuffer CameraData {
	float4x4 viewProjection;
	float3 cameraPosition; float pad0;
	float3 cameraDirection;
	float time : TIME;
}

struct Input {
	float3 position : SV_POSITION;
	float3 normal: NORMAL0;
	float3 tangent: NORMAL1;
	float2 uv : TEXCOORD0;
};

struct Output {
	float4 position : SV_POSITION;
	float3 worldPosition : POSITION;
	float2 uv : TEXCOORD0;
	float3x3 matTBN: NORMAL0;
	float time : TIME;
};

Output main(Input input) {
	Output output = (Output)0;

	output.position = mul( mul( float4(input.position, 1.0f), model), viewProjection );
	output.worldPosition = mul( float4(input.position, 1.0f), model).xyz;

	output.uv = input.uv;

	float3 normal  = normalize( mul( float4(input.normal,  1.0f), model) ).xyz;
	float3 tangent = normalize( mul( float4(input.tangent, 1.0f), model) ).xyz;
	float3 bitangent = cross( normal, tangent);

	output.matTBN = float3x3(tangent, bitangent, normal);

	output.time = time;

	return output;
}