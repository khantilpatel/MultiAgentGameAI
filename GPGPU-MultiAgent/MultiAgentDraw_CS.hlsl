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
struct Agent
{
	int agentId;
	uint pathCount;
	int2 sourceLoc;
	int currentInterpolationId;
	int status; // Agent Status: Stay at initial position 0, Move 1, No Path found 2;
	float velocity;
	float u; // Interpolation parameter
	float randomFactor_X;
	float randomFactor_Z;
	//uint type; // Type: 0 Agent; 1 Collision box
};
struct Node{
	uint x;
	uint y;
};

//struct ComputeShaderOut
//{
//	float3 position;
//	uint type; // Type: 0 Agent; 1 Collision box
//};

//Texture2D<float3> m_map : register(t0);
StructuredBuffer<float3> g_GridCenterListInput : register(t0);
RWStructuredBuffer<float3> bufferOut: register(u0);
RWStructuredBuffer<Agent> agentList: register(u1);
RWStructuredBuffer<uint> pathList: register(u2);

#define NUM_THREAD_X 32
#define NUM_THREAD_Y 32
#define NUM_GRID_BLOCK_X 2
#define MAP_DIMENSIONS 8

[numthreads(NUM_THREAD_X, NUM_THREAD_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint gridId = ((NUM_THREAD_X * NUM_GRID_BLOCK_X) * DTid.y) + DTid.x;
	uint offset = gridId * (MAP_DIMENSIONS * MAP_DIMENSIONS);
	// For testing take this i manually later make according DispatchId
	uint agent_Id = gridId;

	Agent agent = agentList[agent_Id];
	
		uint interpol_id = agent.currentInterpolationId; // + offset later Will Make it.
		if (interpol_id < agent.pathCount)
		{
			if (interpol_id + 1 < agent.pathCount){
				agentList[agent_Id].u = agentList[agent_Id].u + frameTime;
			}
			else
			{
				agentList[agent_Id].u = 1;
			}
			if (agentList[agent_Id].u >= 1){
				agentList[agent_Id].u = 0;
				if (interpol_id + 1 < agent.pathCount){
					interpol_id = interpol_id + 1;
					agentList[agent_Id].currentInterpolationId = interpol_id;
				}
			}
			float u = agentList[agent_Id].u;
			// Get Grid Id for interpolation
			uint interpol_A = pathList[offset + interpol_id];
			uint interpol_B = pathList[offset + (interpol_id + 1)];
			// Get 3d Coordinates to draw on floor
			float3 coord_A = g_GridCenterListInput[interpol_A];
				float3 coord_B = g_GridCenterListInput[interpol_B];
				float acc = 0.5;
			float x = coord_A.x + u * (coord_B.x - coord_A.x);
			float z = coord_A.z + u * (coord_B.z - coord_A.z);

			bufferOut[agent_Id] = float3(x + agent.randomFactor_X, coord_A.y, z + agent.randomFactor_Z);
			
			//bufferOut[0] = g_GridCenterListInput[2];
		}


	//for (uint i = 0; i < 64; i++)
	//{
	//	agentList[i].currentInterpolationId = agentList[i].currentInterpolationId + 1;

	//	if (agentList[i].u >= 1){
	//		agentList[i].u = 0;
	//	}
	//	agentList[i].u = agentList[i].u + frameTime;
	//}
	//bufferOut[0].x = counter;
}

//
//uint counter = 0;
//for (uint i = 0; i < 8; i++)
//{
//	for (uint j = 0; j < 8; j++)
//	{
//		bufferOut[counter] = g_GridCenterListInput[counter];
//		counter++;
//	}
//}
//
//for (uint i = 0; i < 64; i++)
//{
//	agentList[i].currentInterpolationId = agentList[i].currentInterpolationId + 1;
//}