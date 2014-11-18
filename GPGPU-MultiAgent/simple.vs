////////////////////////////////////////////////////////////////////////////////
// Filename: simple.vs
// Description: This is a simple vertex-shader which draws triangle with texture 
// without any changes to the geometry.
////////////////////////////////////////////////////////////////////////////////



struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


PixelInputType SimpleVertexShader(PixelInputType input )
{
	return input;
}


