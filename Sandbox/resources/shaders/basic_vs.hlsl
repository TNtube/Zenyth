cbuffer ModelData {
	float4x4 model;
};

cbuffer CameraData {
	float4x4 view;
	float4x4 projection;
	float3 cameraPosition; float pad0;
	float3 cameraDirection;
	float time : TIME;
}

struct Input {
	float4 position : SV_POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
};

struct Output {
	float4 position : SV_POSITION;
	float4 worldPosition : POSITION;
	float4 normal: NORMAL0;
	float2 uv : TEXCOORD0;
	float3 viewDir: TEXCOORD1;
	float3 camPos: TEXCOORD2;
	float3 camDir: TEXCOORD3;
	float time : TIME;
};

Output main(Input input) {
	Output output = (Output)0;
	output.worldPosition = mul( input.position, model );
	output.position = mul( input.position, model );
	output.position = mul( output.position, view );
	output.position = mul( output.position, projection );
	output.normal = normalize(mul( input.normal, model ));
	output.uv = input.uv;
	output.viewDir = normalize(cameraPosition - output.position.xyz);
	output.camDir = normalize(cameraDirection);
	output.camPos = cameraPosition;
	output.time = time;

	return output;
}