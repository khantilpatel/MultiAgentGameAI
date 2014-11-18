////////////////////////////////////////////////////////////////////////////////
// Filename: texture.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 instancePosition : TEXCOORD1;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Update the position of the vertices based on the data for this particular instance.
    input.position.x = input.instancePosition.x;
    input.position.y = input.instancePosition.y;
    input.position.z = input.instancePosition.z;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
    return output;
}


[maxvertexcount(3)]
void TextureGeometryShader(point PixelInputType input[1], inout TriangleStream<PixelInputType> OutputStream)
{
	// The point passed in is in the horizontal center of the billboard, and at the bottom vertically. Because of this,
	// we will take the trees width and divide it by two when finding the x axis for the quads vertices.
	//float halfWidth = treeBillWidth / 2.0f;

	// The billboard will only be rotated on the y axis, so it's up vector will always be 0,1,0. Because of this, we can
	// find the billboards vertices using the cameras position and the billboards position. We start by getting the billboards
	// plane normal:
	//float3 planeNormal = input[0].worldPos - camPos;
	//planeNormal.y = 0.0f;
	//planeNormal = normalize(planeNormal);

///	float3 upVector = float3(0.0f, 1.0f, 0.0f);

	// Now we need to find the billboards right vector, so we can easily find the billboards vertices from the input point
	//float3 rightVector = normalize(cross(planeNormal, upVector)); // Cross planes normal with the up vector (+y) to get billboards right vector

	//rightVector = rightVector * halfWidth; // change the length of the right vector to be half the width of the billboard

	// Get the billboards "height" vector which will be used to find the top two vertices in the billboard quad
	//upVector = float3(0, treeBillHeight, 0);

	// Create the billboards quad
	float3 vert[3];

	// We get the points by using the billboards right vector and the billboards height
	//vert[0] = input[0].worldPos - rightVector; // Get bottom left vertex
	//vert[1] = input[0].worldPos + rightVector; // Get bottom right vertex
	//vert[2] = input[0].worldPos - rightVector + upVector; // Get top left vertex
	//vert[3] = input[0].worldPos + rightVector + upVector; // Get top right vertex

	
	float3 leftRightVector = float3(1.0f, 0.0f, 0.0f);

	float3 upVector = float3(0.0f, 2.0f, 0.0f);

	vert[0] = input[0].position - leftRightVector;
	vert[1] = input[0].position + upVector;
	vert[2] = input[0].position + leftRightVector;

	//vert[0] = float3(-1.0f, -1.0f, -3.0f);   //input[0].position - leftRightVector;
	//vert[1] = float3(0.0f, 1.0f, -3.0f);		// input[0].position + upVector;
	//vert[2] = float3(1.0f, -1.0f, -3.0f);		//input[0].position + leftRightVector;



	// Get billboards texture coordinates
	float2 texCoord[3];
	texCoord[0] = float2(0, 1);
	texCoord[1] = float2(0.5, 0);
	texCoord[2] = float2(1, 1);
	//texCoord[3] = float2(1, 0);

	// Now we "append" or add the vertices to the outgoing stream list
	//VS_OUTPUT outputVert;
	PixelInputType output;
	for(int i = 0; i < 3; i++)
	{
	    //outputVert.Pos = mul(float4(vert[i], 1.0f), WVP);
		//outputVert.worldPos = float4(vert[i], 0.0f);
		//outputVert.TexCoord = texCoord[i];

		// These will not be used for billboards
		//outputVert.normal = float3(0,0,0);
		//outputVert.tangent = float3(0,0,0);

		//OutputStream.Append(outputVert);

		/////
		output.position.x = vert[i].x;
		output.position.y = vert[i].y;
		output.position.z = vert[i].z;
		output.position.w = 1.0f;

	   output.position = mul(output.position, worldMatrix);
	   output.position = mul(output.position, viewMatrix);
	   output.position = mul(output.position, projectionMatrix);

		output.tex = texCoord[i];

		OutputStream.Append(output);
	}
}