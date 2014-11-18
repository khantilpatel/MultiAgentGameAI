
struct GSOutput
{
	float4 pos : SV_POSITION;
};

Texture2D shaderTexture ;
SamplerState SampleType;

struct VertexOut
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;	
};


float4 main(VertexOut input) : SV_TARGET
{
	float4 textureColor = float4(0.0, 0.0, 0.0, 0.0);

	textureColor = shaderTexture.Sample(SampleType, input.tex);

	return textureColor;

	//return float4(0.5f, 0.5f, 1.0f, 1.0f);
}