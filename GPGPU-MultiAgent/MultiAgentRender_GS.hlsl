

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	float4x4 gTexTransform;
	
};

struct VertexOut
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


[maxvertexcount(36)]
void main(
	point float4 input[1] : SV_POSITION,
	inout TriangleStream< VertexOut > output)
{
		//http://faculty.ycp.edu/~dbabcock/PastCourses/cs470/labs/lab04.html
	
	float scale = 0;
	float halfscale= 0;
	
		scale = 0.3;
		halfscale = scale *0.5;

		
		float4x4 WorldViewProj;
		float r_angle = radians(20);
		float my_sin = sin(r_angle);
		float my_cos = cos(r_angle);



		float4x4 rotationMat = float4x4(
			my_cos, 0, -my_sin, 0,
			0, 1, 0, 0 ,
			my_sin, 0, my_cos, 0 ,
			0, 0, 0, 1 
		);

		float4x4 transMat = float4x4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			input[0].x, input[0].y, input[0].z, 1
			);

		WorldViewProj = mul(rotationMat, transMat);
		WorldViewProj = mul(WorldViewProj, viewMatrix);
		WorldViewProj = mul(WorldViewProj, projectionMatrix);

		//WorldViewProj = mul(rotationMat, WorldViewProj);
		//////////////////////////////////////////////////////////////////////////
		//element0.position = 



		float4 local0 = float4(-halfscale, 0, halfscale, 1);
		float4 local1 = float4(-halfscale, scale, halfscale, 1);
		float4 local2 = float4(halfscale, scale, halfscale, 1);
		float4 local3 = float4(halfscale, 0, halfscale, 1);
		float4 local4 = float4(-halfscale, 0, -halfscale, 1);
		float4 local5 = float4(-halfscale, scale, -halfscale, 1);
		float4 local6 = float4(halfscale, scale, -halfscale, 1);
		float4 local7 = float4(halfscale, 0, -halfscale, 1);

		//Left 0
		VertexOut element0;
		element0.position = mul(local0, WorldViewProj);
		element0.tex.x = 0.0f;
		element0.tex.y = 0.0f;

		//Top-left 1
		VertexOut element1;

		element1.position = mul(local1, WorldViewProj);
		element1.tex.x = 0.0f;
		element1.tex.y = 1.0f;

		// Top-Right 2
		VertexOut element2;
		element2.position = mul(local2, WorldViewProj);
		element2.tex.x = 1.0f;
		element2.tex.y = 1.0f;

		// Vertex 3
		VertexOut element3;	
		element3.position = mul(local3, WorldViewProj);
		element3.tex.x = 1.0f;
		element3.tex.y = 0.0f;
		////////////////////////////////////////
		// 4
		VertexOut element4;
		element4.position = mul(local4, WorldViewProj);
		element4.tex.x = 0.0f;
		element4.tex.y = 0.0f;

		// 5
		VertexOut element5;
		element5.position = mul(local5, WorldViewProj);
		element5.tex.x = 0.0f;
		element5.tex.y = 1.0f;

		// 6
		VertexOut element6;
		element6.position = mul(local6, WorldViewProj);
		element6.tex.x = 1.0f;
		element6.tex.y = 1.0f;

		// 7
		VertexOut element7;	
		element7.position = mul(local7, WorldViewProj);
		element7.tex.x = 1.0f;
		element7.tex.y = 0.0f;
		////Front Face
		output.Append(element0);
		output.Append(element1);
		output.Append(element2);

		

		output.Append(element0);
		output.Append(element2);
		output.Append(element3);

		output.RestartStrip();
		//////Back Face
		output.Append(element4);
		output.Append(element6);
		output.Append(element5);

		output.Append(element4);
		output.Append(element7);
		output.Append(element6);

		output.RestartStrip();
		//Left Face
		element4.tex.x = 0.0f;
		element4.tex.y = 0.0f;
		output.Append(element4);

		element5.tex.x = 0.0f;
		element5.tex.y = 1.0f;
		output.Append(element5);

		element1.tex.x = 1.0f;
		element1.tex.y = 1.0f;
		output.Append(element1);

		output.Append(element4);
		output.Append(element1);
		element0.tex.x = 1.0f;
		element0.tex.y = 0.0f;
		output.Append(element0);

		output.RestartStrip();
		////Right Face
		element3.tex.x = 0.0f;
		element3.tex.y = 0.0f;
		output.Append(element3);
		element2.tex.x = 0.0f;
		element2.tex.y = 1.0f;
		output.Append(element2);
		element6.tex.x = 1.0f;
		element6.tex.y = 1.0f;
		output.Append(element6);

		//output.RestartStrip();

		output.Append(element3);
		output.Append(element6);
		element6.tex.x = 1.0f;
		element6.tex.y = 0.0f;
		output.Append(element7);

		output.RestartStrip();
		////Top Face
		element1.tex.x = 0.0f;
		element1.tex.y = 0.0f;
		output.Append(element1);
		element5.tex.x = 0.0f;
		element5.tex.y = 1.0f;
		output.Append(element5);
		element6.tex.x = 1.0f;
		element6.tex.y = 1.0f;
		output.Append(element6);

		//output.RestartStrip();

		output.Append(element1);
		output.Append(element6);
		element2.tex.x = 1.0f;
		element2.tex.y = 0.0f;
		output.Append(element2);

		output.RestartStrip();
		//Bottom
		element4.tex.x = 0.0f;
		element4.tex.y = 0.0f;
		output.Append(element4);
		element0.tex.x = 1.0f;
		element0.tex.y = 0.0f;
		output.Append(element0);
		element3.tex.x = 1.0f;
		element3.tex.y = 1.0f;
		output.Append(element3);

		//output.RestartStrip();

		output.Append(element4);
		output.Append(element3);
		element6.tex.x = 0.0f;
		element6.tex.y = 1.0f;
		output.Append(element7);

		output.RestartStrip();
		//
		////////////////////////////////////////
		////Right
		//element.position.x = input[0].x + halfscale;
		//element.position.y = input[0].y ;
		//element.position.z = input[0].z - halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 1.0f;
		//element.tex.y = 0.0f;
		//output.Append(element);
	
		////Top-left
		//element.position.x = input[0].x - halfscale;
		//element.position.y = input[0].y + scale;
		//element.position.z = input[0].z - halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 0.0f;
		//element.tex.y = 1.0f;
		//output.Append(element);
		/////////////////////////////////////////////////////////////////
		//// Front
		////Right
		//element.position.x = input[0].x + 0.5;
		//element.position.y = input[0].y;
		//element.position.z = input[0].z + halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 1.0f;
		//element.tex.y = 0.0f;
		//output.Append(element);
		//// Left		
		//element.position.x = input[0].x - halfscale;
		//element.position.y = input[0].y;
		//element.position.z = input[0].z + halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 0.0f;
		//element.tex.y = 0.0f;
		//output.Append(element);
		////Top-Right
		//element.position.x = input[0].x + halfscale;
		//element.position.y = input[0].y + scale;
		//element.position.z = input[0].z + halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 1.0f;
		//element.tex.y = 1.0f;
		//output.Append(element);
		////Top-Left
		//element.position.x = input[0].x - halfscale;
		//element.position.y = input[0].y + scale;
		//element.position.z = input[0].z + halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 0.0f;
		//element.tex.y = 1.0f;
		//output.Append(element);

		//////////////////////////////////////////////////////////////////////////
		//// Right
		//element.position.x = input[0].x + 0.5;
		//element.position.y = input[0].y;
		//element.position.z = input[0].z - halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 1.0f;
		//element.tex.y = 0.0f;
		//output.Append(element);
		//// Left		
		//element.position.x = input[0].x + 0.5;
		//element.position.y = input[0].y;
		//element.position.z = input[0].z + halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 1.0f;
		//element.tex.y = 0.0f;
		//output.Append(element);
		////Top-Right
		//element.position.x = input[0].x + halfscale;
		//element.position.y = input[0].y + scale;
		//element.position.z = input[0].z - halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 1.0f;
		//element.tex.y = 1.0f;
		//output.Append(element);
		////Top-Left
		//element.position.x = input[0].x + halfscale;
		//element.position.y = input[0].y + scale;
		//element.position.z = input[0].z + halfscale;
		//element.position.w = input[0].w;
		//element.position = mul(element.position, worldMatrix);
		//element.position = mul(element.position, viewMatrix);
		//element.position = mul(element.position, projectionMatrix);
		//element.tex.x = 1.0f;
		//element.tex.y = 1.0f;
		//output.Append(element);
}