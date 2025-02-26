cbuffer ModelData : register(b0) {
	float4x4 model;
};

cbuffer CameraData : register(b1) {
	float4x4 view;
	float4x4 projection;
	float3 cameraPosition;
	float time : TIME;
}

struct Input {
	float4 position : POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
};

struct Output {
	float4 position : SV_POSITION;
	float4 worldPosition : POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
	float3 viewDir: TEXCOORD1;
	float time : TIME;
};

Output main(Input input) {
	Output output = (Output)0;
	output.worldPosition = mul( input.position, model );
	output.position = mul( input.position, model );
	output.position = mul( output.position, view );
	output.position = mul( output.position, projection );
	output.normal = mul( input.normal, model );
	output.uv = input.uv;
	output.viewDir = cameraPosition - output.position.xyz;
	output.time = time;

	return output;
}