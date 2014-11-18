
float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}


//***********************************************
// DRAW TECH                                    *
//***********************************************

#define PT_EMITTER 0
#define PT_FLARE 1

Texture2DArray gTexArray;

SamplerState samLinear;

cbuffer cbPerFrame
{
	float4x4 gViewProj;	
	float3 gEyePosW;
	float gap;
};

cbuffer cbFixed
{
	// Net constant acceleration used to accerlate the particles.
	float3 gAccelW = float3(0.0f, 7.8f, 0.0f);
	
	// Texture coordinates used to stretch texture over quad 
	// when we expand point particle into a quad.
	float2 gQuadTexC[4] = 
	{
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};
};



struct Particle
{
	float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW       : SIZE;
	float Age          : AGE;
	uint Type          : TYPE;
};
struct VertexOut
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
	uint   Type  : TYPE;
};

VertexOut DrawVS(Particle vin)
{
	VertexOut vout;
	
	float t = vin.Age;
	
	// constant acceleration equation
	vout.PosW = 0.5f*t*t*float3(0.0f, 7.8f, 0.0f) + t*vin.InitialVelW + vin.InitialPosW;
	
	// fade color with time
	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t/1.0f);
	vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);
	
	vout.SizeW = vin.SizeW;
	vout.Type  = vin.Type;
	
	return vout;
}

struct GeoOut
{
	float4 PosH  : SV_Position;
	float4 Color : COLOR;
	float2 Tex   : TEXCOORD;
};

// The draw GS just expands points into camera facing quads.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1], 
            inout TriangleStream<GeoOut> triStream)
{	
	// do not draw emitter particles.
	if( gin[0].Type != PT_EMITTER )
	{
		//
		// Compute world matrix so that billboard faces the camera.
		//
		float3 look  = normalize(gEyePosW.xyz - gin[0].PosW);
		float3 right = normalize(cross(float3(0,1,0), look));
		float3 up    = cross(look, right);
		
		//
		// Compute triangle strip vertices (quad) in world space.
		//
		float halfWidth  = 0.5f*gin[0].SizeW.x;
		float halfHeight = 0.5f*gin[0].SizeW.y;
	
		float4 v[4];
		float4 v0 = float4(gin[0].PosW + halfWidth*right - halfHeight*up, 1.0f);
		float4 v1 = float4(gin[0].PosW + halfWidth*right + halfHeight*up, 1.0f);
		float4 v2 = float4(gin[0].PosW - halfWidth*right - halfHeight*up, 1.0f);
		float4 v3 = float4(gin[0].PosW - halfWidth*right + halfHeight*up, 1.0f);
		
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		v[3] = v3;
		
		float2 gQuadTexC_Test[4];
	
		gQuadTexC_Test[0] = float2(0.0f, 1.0f);
		gQuadTexC_Test[1] = float2(1.0f, 1.0f);
		gQuadTexC_Test[2] = float2(0.0f, 0.0f);
		gQuadTexC_Test[3] = float2(1.0f, 0.0f);
	

		//
		// Transform quad vertices to world space and output 
		// them as a triangle strip.
		//
		GeoOut gout;
		[unroll]
		for(int i = 0; i < 4; ++i)
		{
			float4 pos = mul(v[i], gViewProj);
			gout.PosH  = pos;
			gout.Tex   = gQuadTexC_Test[i];
			gout.Color = gin[0].Color;
			triStream.Append(gout);
		}	
	}
}

SamplerState samLinearTest
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

float4 DrawPS(GeoOut pin) : SV_TARGET
{
	return gTexArray.Sample(samLinear, float3(pin.Tex, 0))*pin.Color;
}
