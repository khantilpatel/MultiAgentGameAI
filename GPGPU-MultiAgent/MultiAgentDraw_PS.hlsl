
////////////////////////////////////////////////////////////////////////////////
// Filename: particle.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////

Texture2D shaderTexture;
SamplerState SampleType;


struct VertexOut
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 NormalW : NORMAL;
};


float4 main(VertexOut input) : SV_TARGET
{
	float4 textureColor = shaderTexture.Sample(SampleType, input.tex);

	return textureColor;
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
}