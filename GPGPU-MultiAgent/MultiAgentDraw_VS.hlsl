////////////////////////////////////////////////////////////////////////////////
// Filename: MultiAgentDraw_VS.vs
////////////////////////////////////////////////////////////////////////////////



/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	float4x4 gTexTransform;
	float frameTime;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 NormalW : NORMAL;	
};

VertexOut main(VertexIn input)
{
	VertexOut output;
	float4 pos = float4(input.PosL, 1.0f);

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(pos, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex =  mul(float4(input.Tex, 0.0f, 1.0f), gTexTransform).xy; 

	output.NormalW = float3(0.0,0.0,0.0);
	// Store the particle color for the pixel shader. 
	//output.color = input.color;

	return output;
}