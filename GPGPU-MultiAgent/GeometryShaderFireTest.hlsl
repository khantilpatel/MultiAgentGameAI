#define PT_EMITTER 0
#define PT_FLARE 1

cbuffer cbPerFrame
{
	float1 gFrameTime;	
	float3 gEmitPosition;
	float1 gGameTime;
	float3 gEyePosition;
};

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

float3 RandUnitVec3(float offset)
{
	float u = (gGameTime + offset);

	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

		return normalize(v);
}

float3 RandVec3(float offset)
{
	float u = (gGameTime + offset);

	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

		return v;
}

[maxvertexcount(46)]
void main(
	point GSOutput input[1], 
	inout PointStream< GSOutput > output
	)
{
	input[0].Age += gFrameTime;
	if( input[0].Type == PT_EMITTER )
	{	
		if( input[0].Age > 0.002f )
		{
			for(uint i = 0; i < 40; ++i)
			{

				float3 vRandom = 35.0f*RandVec3((float)i/50.0f);
				vRandom.y = 500.0f;

				GSOutput p;
				p.InitialPosW = gEyePosition.xyz + vRandom;			
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


//float4x4 gViewProj;	
//float3 gEyePosW;
//float gap;

//float1 gGameTime;


//	GSOutput element;
//	element.InitialPosW = gEmitPosition;
//	element.InitialVelW =gEyePosition; //35.0f*RandVec3((float)i/5.0f);
//	element.SizeW =float2(gGameTime,0.0);
//	element.Type = 1;
//	element.Age = gFrameTime;
////	float testframe = gGameTime;
//	output.Append(element);
/////////////////////////////


	//for (uint i = 0; i < 6; i++)
	//{
	//	GSOutput element;
	//	element.InitialPosW = gEmitPosition;
	//	element.InitialVelW =gEyePosition; //35.0f*RandVec3((float)i/5.0f);
	//	element.SizeW =float2(gGameTime,0.0);
	//	element.Type = 1;
	//	element.Age = gFrameTime;
	//	//output.Append(element);
	//}