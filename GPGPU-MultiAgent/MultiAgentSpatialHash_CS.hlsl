


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
	uint seed;
	//uint type; // Type: 0 Agent; 1 Collision box
};


RWByteAddressBuffer SpatialIndexTableBuffer: register(u0);
RWStructuredBuffer<uint> SpatialAgentIdTableBuffer: register(u1);
RWStructuredBuffer<Agent> agentList: register(u2);

#define NUM_THREAD_X 32
#define NUM_THREAD_Y 32
#define NUM_GRID_BLOCK_X 2
#define MAP_DIMENSIONS 8
#define CUBE_SIZE 2.5 // Width/height in pixel?
#define UINT_SIZE 4
#define NUM_AGENTS_PER_BLOCK 5

groupshared int shared_data[MAP_DIMENSIONS*MAP_DIMENSIONS];

[numthreads(NUM_THREAD_X, NUM_THREAD_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	
	uint gridId = ((NUM_THREAD_X * NUM_GRID_BLOCK_X) * DTid.y) + DTid.x;
	uint offset = gridId * (MAP_DIMENSIONS * MAP_DIMENSIONS);
	// For testing take this i manually later make according DispatchId
	uint agent_Id = gridId;

	float3 m_current_position = float3(0.0, 0.0, 0.0);
	Agent agent = agentList[agent_Id];

	int hash_index = floor(agent.current_position.x / CUBE_SIZE) +
		MAP_DIMENSIONS *(floor(agent.current_position.z / CUBE_SIZE));

	int agent_hash_index;
	SpatialIndexTableBuffer.InterlockedAdd(hash_index*UINT_SIZE, 1, agent_hash_index);

	if (agent_hash_index < NUM_AGENTS_PER_BLOCK){
		SpatialAgentIdTableBuffer[(hash_index*NUM_AGENTS_PER_BLOCK) + agent_hash_index] = agent_Id;
	}
}