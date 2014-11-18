#define PT_EMITTER 0
#define PT_FLARE 1

struct GSOutput
{
	float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW       : SIZE;
	float Age          : AGE;
	uint Type          : TYPE;
};

Texture1D gRandomTex;
SamplerState samLinear;

cbuffer cbPerFrame
{
	float gTimeStepT;
	float3 gEmitPosWT;
	float gGameTimeT;
	float3 gEyePosWT;
};

float3 RandUnitVec3(float offset)
{
	//float u = (gGameTimeT + offset);
	
	float3 v = gRandomTex.SampleLevel(samLinear, 0.0f, 0).xyz;
	
	return normalize(v);
}

float3 RandVec3(float offset)
{
	float u = (gGameTimeT + offset);
	
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	return v;
}

[maxvertexcount(6)]
void main(
	point GSOutput input[1], 
	inout PointStream< GSOutput > output
)
{
	//for (uint i = 0; i < 3; i++)
	//{
	//	GSOutput element;
	//	element.pos = input[i];
	//	output.Append(element);
	//}
	input[0].Age += gTimeStepT;
	if( input[0].Type == PT_EMITTER )
	{	
		if( input[0].Age > 0.002f )
		{
			for(int i = 0; i < 5; ++i)
			{
				float3 vRandom = 35.0f*RandVec3((float)i/5.0f);
				vRandom.y = 20.0f;
			
				GSOutput p;
				p.InitialPosW = gEyePosWT.xyz ;//+ vRandom;
				float3 pos = gEyePosWT;
					pos.z = pos.z + 10;
				p.InitialPosW = gEyePosWT;
				p.InitialVelW = float3(0.0f, 0.0f, 0.0f);
				p.SizeW       = float2(1.0f, 1.0f);
				p.Age         = 0.0f;
				p.Type        = PT_FLARE;
			
				output.Append(p);
			}
			// reset the time to emit
			input[0].Age = 0.0f;
		}
		
		// always keep emitters
		output.Append(input[0]);
		
	}
	else
	{
		// Specify conditions to keep particle; this may vary from system to system.
		if( input[0].Age <= 3.0f )
			output.Append(input[0]);
	}
}