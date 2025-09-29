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
	float3 position : SV_POSITION;
	float3 normal: NORMAL0;
	float3 tangent: NORMAL1;
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
	float4 pos = float4(input.position, 1.0f);
	float4 norm = float4(input.normal, 1.0f);
	output.worldPosition = mul( pos, model );
	output.position = mul( pos, model );
	output.position = mul( output.position, view );
	output.position = mul( output.position, projection );
	output.normal = normalize(mul( norm, model ));
	output.uv = input.uv;
	output.viewDir = normalize(cameraPosition - output.position.xyz);
	output.camDir = normalize(cameraDirection);
	output.camPos = cameraPosition;
	output.time = time;

	return output;
}