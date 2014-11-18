

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

float4 main(float3 posIn: POSITION) :SV_POSITION
{
	
	float4 position = float4(posIn, 1.0f);
	// Calculate the position of the vertex against the world, view, and projection matrices.
	/*element.position = mul(element.position, worldMatrix);
	element.position = mul(element.position, viewMatrix);
	element.position = mul(element.position, projectionMatrix);*/
	return position;
}