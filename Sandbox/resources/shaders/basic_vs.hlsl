cbuffer ModelData : register(b0) {
	float4x4 model;
};

cbuffer CameraData : register(b1) {
	float4x4 view;
	float4x4 projection;
}

struct Input {
	float4 position : POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
};

struct Output {
	float4 position : SV_POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
};

Output main(Input input) {
	Output output = (Output)0;
	output.position = mul( input.position, model );
	output.position = mul( output.position, view );
	output.position = mul( output.position, projection );
	output.normal = input.normal;
	output.uv = input.uv;

	return output;
}