cbuffer ObjectData : register(b0) {
	float4x4 worldMatrix;
	// add material properties ?
};

cbuffer CameraData : register(b1) {
	float4x4 viewProjection;
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
};

Output main(Input input) {
	Output output = (Output)0;

	output.position = mul( mul( float4(input.position, 1.0f), worldMatrix), viewProjection );
	output.worldPosition = mul( float4(input.position, 1.0f), worldMatrix).xyz;
	output.uv = input.uv;

	float3 normal  = normalize( mul( float4(input.normal,  1.0f), worldMatrix) ).xyz;
	float3 tangent = normalize( mul( float4(input.tangent, 1.0f), worldMatrix) ).xyz;
	float3 bitangent = cross( normal, tangent);

	output.matTBN = float3x3(tangent, bitangent, normal);

	return output;
}