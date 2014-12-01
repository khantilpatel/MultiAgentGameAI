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
struct vec2
{


};
struct Agent
{
	float3 current_position;
	int agentId;
	uint pathCount;
	int2 sourceLoc;
	int currentInterpolationId;
	int status; // Agent Status: Stay at initial position 0, Move 1, No Path found 2;
	float velocity;
	float isFirstPass; // Interpolation parameter
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
RWStructuredBuffer<float3> g_AgentCurrentPositionInput : register(u3);

RWByteAddressBuffer SpatialIndexTableBuffer: register(u4);
RWStructuredBuffer<uint> SpatialAgentIdTableBuffer: register(u5);



#define NUM_THREAD_X 32
#define NUM_THREAD_Y 32
#define NUM_GRID_BLOCK_X 2
#define MAP_DIMENSIONS 8
#define CUBE_SIZE 2.5 // Width/height in pixel?
#define NUM_AGENTS_PER_BLOCK 5
#define AGENT_WIDTH 0.8
#define UINT_SIZE 4
//#define INFINITY +INF
static const float	INFINITY = 1e6;

[numthreads(NUM_THREAD_X, NUM_THREAD_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int agentId[20];

	uint gridId = ((NUM_THREAD_X * NUM_GRID_BLOCK_X) * DTid.y) + DTid.x;
	uint offset = gridId * (MAP_DIMENSIONS * MAP_DIMENSIONS);
	// For testing take this i manually later make according DispatchId
	uint agent_Id = gridId;

	float test = INFINITY;
	bool test_inf = isinf(test);

	float3 m_current_position = float3(0.0, 0.0, 0.0);
		Agent agent = agentList[agent_Id];

	agentId[2] = agent_Id;

	uint interpol_id = agent.currentInterpolationId; // + offset later Will Make it.
	if (interpol_id < agent.pathCount)
	{
		uint interpol_A = pathList[offset + interpol_id];
		float3 coord_A = g_GridCenterListInput[interpol_A];
			// only for the first node of 
			if (agent.isFirstPass < 2){
				m_current_position = coord_A;
				agentList[agent_Id].isFirstPass += 1;
			}
			else{
				m_current_position = agent.current_position;
				//float3(agentList[agent_Id].current_position_x, agentList[agent_Id].current_position_y, 
				//agentList[agent_Id].current_position_z);
			}
			uint interpol_B = pathList[offset + (interpol_id + 1)];
			float3 coord_B = g_GridCenterListInput[interpol_B];

			float temp_distance = distance(m_current_position, coord_B);

			if (CUBE_SIZE * 0.5 > temp_distance && interpol_id + 2 < agent.pathCount){
				interpol_id = interpol_id + 1;
				agentList[agent_Id].currentInterpolationId = interpol_id;
				interpol_B = pathList[offset + (interpol_id + 1)];
				coord_B = g_GridCenterListInput[interpol_B];
			}
			float3 temp = coord_B - m_current_position;
			
			float3 prefVelocity = normalize(temp) * agent.velocity;
			
			int hash_index = floor(agent.current_position.x / CUBE_SIZE) +
			MAP_DIMENSIONS *(floor(agent.current_position.z / CUBE_SIZE));

			bool isCollision = false;

			int num_neighbours = SpatialIndexTableBuffer.Load(hash_index*UINT_SIZE);

			for (int i = 0; i < num_neighbours; i++)
			{
				int index_agent = (hash_index * NUM_AGENTS_PER_BLOCK) + i;
				int agent_id_neighbour = SpatialAgentIdTableBuffer[index_agent];

				float dist = distance(m_current_position, agentList[agent_id_neighbour].current_position);

				if (dist < sqrt((AGENT_WIDTH*AGENT_WIDTH) + (AGENT_WIDTH*AGENT_WIDTH)))
				{
					isCollision = true;
				}
			}

			m_current_position += normalize(temp) * agent.velocity * frameTime;

			agentList[agent_Id].current_position = m_current_position; 
		
			bufferOut[agent_Id] = float3(m_current_position.x + agent.randomFactor_X,
				coord_B.y, m_current_position.z + agent.randomFactor_Z);
	}

	/*	if (interpol_id + 1 < agent.pathCount){
	agentList[agent_Id].u = agentList[agent_Id].u + frameTime;
	}
	else
	{
	agentList[agent_Id].u = 1;
	}*/
	/*		if (agentList[agent_Id].u >= 1){
	agentList[agent_Id].u = 0;

	}*/
	//float u = agentList[agent_Id].u;
	//// Get Grid Id for interpolation

	//float acc = 0.5;
	//float x = coord_A.x + u * (coord_B.x - coord_A.x);
	//float z = coord_A.z + u * (coord_B.z - coord_A.z);



	//bufferOut[0] = g_GridCenterListInput[2];



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

//*****Steps to implement the ROV***************
//1. get the count of agents in current Grid
//2. for loop with all neighbour except itself and check if collided.
//3. call the function to simulateNewVelocity
// agentPosition, Radius, prefVelocity,
// isCOllision, timestep
// Neighbour agent list
//4. For loop to simulate random velocity samples.
//5. Simulate Random velocity function
//6. check for TimetoCollision function
//7. get the best velocity and terminate 

//*********************************************


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