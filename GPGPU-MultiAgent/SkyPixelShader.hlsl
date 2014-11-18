float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}


//cbuffer cbPerFrame
//{
//	float4x4 gWorldViewProj;
//};

cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};
 
// Nonnumeric values cannot be added to a cbuffer.
TextureCube gCubeMap;

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosH = mul(float4(vin.PosL, 1.0f), worldMatrix);
	vout.PosH = mul(vout.PosH, viewMatrix);
	vout.PosH = mul(vout.PosH, projectionMatrix);
	vout.PosH = vout.PosH.xyww;

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	//vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj).xyww;
	
	// Use local vertex position as cubemap lookup vector.
	vout.PosL = vin.PosL;
	
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return gCubeMap.Sample(samTriLinearSam, pin.PosL);
}
