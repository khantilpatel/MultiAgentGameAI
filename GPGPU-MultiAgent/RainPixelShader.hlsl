float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//***********************************************
// DRAW TECH                                    *
//***********************************************

#define PT_EMITTER 0
#define PT_FLARE 1

Texture1D gRandomTex: register(b0);
Texture2DArray gTexArray: register(b1);

SamplerState samLinear;

cbuffer cbPerFrame
{
	float4x4 gViewProj;	
	float3 gEyePosW;
	float gap;
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
	uint   Type  : TYPE;
};

float3 RandVec3(float offset)
{
	float u = (gap + offset);
	
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	return v;
}

VertexOut DrawVS(Particle vin)
{
	VertexOut vout;
	
	float t = vin.Age;
	
	// constant acceleration equation
	vout.PosW = 150.0f*t*t*float3(0.0f, -1.0f, 0.0f) + t*vin.InitialVelW +vin.InitialPosW;
	
	vout.Type  = vin.Type;
	
	return vout;
}

struct GeoOut
{
	float4 PosH  : SV_Position;
	float2 Tex   : TEXCOORD;
};

// The draw GS just expands points into camera facing quads.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1], 
            inout LineStream<GeoOut> lineStream)
{	
		// do not draw emitter particles.
	if( gin[0].Type != PT_EMITTER )
	{
		// Slant line in acceleration direction.
		float3 p0 = gin[0].PosW ;
		float3 p1 = gin[0].PosW + 3.0f*float3(0.0f , -1.0f, 0.0f) ;
		
		GeoOut v0;
		v0.PosH = mul(float4(p0, 1.0f), gViewProj);
		v0.Tex = float2(0.0f, 0.0f);
		lineStream.Append(v0);
		
		GeoOut v1;
		v1.PosH = mul(float4(p1, 1.0f), gViewProj);
		v1.Tex  = float2(1.0f, 1.0f);
		lineStream.Append(v1);
	}
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
	return gTexArray.Sample(samLinear, float3(pin.Tex, 0));
}
