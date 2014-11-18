#define PT_EMITTER 0
#define PT_FLARE 1

cbuffer cbPerFrame
{
	float3 gEmitPosW;
	float gTimeStep;
	float3 gEyePosW;
	float gGameTime;
		
};

Texture1D gRandomTex;

SamplerState samLinear;

 
struct GSOutput
{
		float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW       : SIZE;
	float Age          : AGE;
	uint Type          : TYPE;
};



//***********************************************
// HELPER FUNCTIONS                             *
//***********************************************
float3 RandUnitVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);
	
	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	// project onto unit sphere
	return normalize(v);
}

[maxvertexcount(2)]
void main(
	point GSOutput input[1], 
	inout PointStream<GSOutput> output)
{
	/*for (uint i = 0; i < 2; i++)
	{
		float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;

		GSOutput element;
		element.InitialPosW = gEmitPosW;
		element.InitialVelW = 4.0f*vRandom;
		element.SizeW.x = gGameTime;
		element.SizeW.y = gTimeStep;
		element.Age = 0.0f;
		element.Type = PT_FLARE;
		output.Append(element);
	}*/

	input[0].Age += gTimeStep;
	
	if( input[0].Type == PT_EMITTER )
	{	
		// time to emit a new particle?
		if( input[0].Age > 0.005f )
		{
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;
			
			GSOutput p;
			p.InitialPosW = gEmitPosW.xyz;
			p.InitialVelW = 4.0f*vRandom;
			p.SizeW       = float2(3.0f, 3.0f);
			p.Age         = 0.0f;
			p.Type        = PT_FLARE;
			
			output.Append(p);
			
			// reset the time to emit
			input[0].Age = 0.0f;
		}
		
		// always keep emitters
		output.Append(input[0]);
	}
	else
	{
		// Specify conditions to keep particle; this may vary from system to system.
		if( input[0].Age <= 1.0f )
			output.Append(input[0]);
	}		
}