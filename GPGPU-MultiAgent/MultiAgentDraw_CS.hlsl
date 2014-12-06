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
	uint seed;
	float2 velocity_dir;
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
RWStructuredBuffer<float2> testRandomSample: register(u6);


#define NUM_THREAD_X 32
#define NUM_THREAD_Y 32
#define NUM_GRID_BLOCK_X 2
#define MAP_DIMENSIONS 8
#define CUBE_SIZE 2.5 // Width/height in pixel?
#define NUM_AGENTS_PER_BLOCK 10
#define AGENT_WIDTH 0.3
#define UINT_SIZE 4
//#define INFINITY +INF

#define RVO_INFTY 9e9f

#define m_sampleCountDefault 250
#define m_prefSpeedDefault 0.5
#define m_maxSpeedDefault 0.7
#define m_safetyFactorDefault 0.3 // Worked 0.33
#define m_maxAccelDefault 0.3
#define MOORS_CONST 1

float absSq(float2 q)
{
	return q.x * q.x + q.y * q.y;
}

float sqr(float n)
{
	return n*n;
}

float abs_vec(float2 q) {
	return sqrt(absSq(q));
}

float det(float2 p, float2 q) { return p.x*q.y - p.y*q.x; }

float computeTimeToCollison(float2 p, float2 v, float2 p2, float radius, bool collision)
{
	float2 ba = p2 - p;
	float sq_diam = radius * radius;
	float time;

	float discr = -sqr(det(v, ba)) + sq_diam * absSq(v);
	if (discr > 0) {
		if (collision) {
			time = (dot(v, ba) + sqrt(discr)) / absSq(v);

			if (time < 0) {
				time = -RVO_INFTY;
			}
		}
		else {
			time = (dot(v, ba) - sqrt(discr)) / absSq(v);
			if (time < 0) {
				time = RVO_INFTY;
			}
		}
	}
	else {
		if (collision) {
			time = -RVO_INFTY;
		}
		else {
			time = RVO_INFTY;
		}
	}
	return time;
}

float2 simulateNewVelocity(int agent_Id, Agent m_pOwner, float agent_radius, float2 prefVelocity, bool isCollison, float _timeStep)
{
	float2 return_final_velocity ;

	uint N = 32768;
	uint P1 = 1103515245;
	uint P2 = 12345;

	float min_penalty = RVO_INFTY;

	float2 vCand;

	float2 randomData;

	uint new_seed;

	float2 init_random_buffer = float2(m_pOwner.agentId, m_pOwner.seed);

		if (m_pOwner.agentId == 0){
		testRandomSample[0] = init_random_buffer;
		}
	for (int n = 0; n < m_sampleCountDefault; n++)
	{
		if (n == 0) {
			vCand = prefVelocity;
			randomData = vCand;
		}
		else {
			bool test = false;
			do {
				
				randomData = float2(0.0, 0.0);

				new_seed = (P1 * m_pOwner.seed + P2);
				float randX = (new_seed / 65536) % N; //rand();
				randX *= 2.0f;
				randomData.x = new_seed;

				new_seed = (P1 + new_seed + P2);
				float randY = (new_seed / 65536) % N; // rand();
				randY *= 2.0f;
				randomData.y = new_seed;

				// Update the seed into the Buffer
		
				m_pOwner.seed = new_seed;

				float x = randX - N;
				float y = randY - N;

				float rand_max = N - 1;

				vCand = float2(x, y); //2.0f*rand() - RAND_MAX, 2.0f*rand() - RAND_MAX);

				
				float absVCand = absSq(vCand);
				float sqrRand_Max = sqr((float)N);

				test = absVCand > sqrRand_Max;

			} while (test);//absSq(vCand) > sqr((float)N));

			//double max = m_maxSpeedDefault;
			float max1 =  m_maxSpeedDefault;

			//float divide = max / N;
			float divide1 = max1 / N;

			//vCand *= divide;
			vCand *= divide1;			
		}

		if (m_pOwner.agentId == 0){
			testRandomSample[n + 1] = randomData;// vCand;
		}

		float dV; // distance between candidate velocity and preferred velocity
		if (isCollison) {
			dV = 0;
		}
		else {
			dV = abs_vec(vCand - prefVelocity);
		}


		// searching for smallest time to collision
		float ct = RVO_INFTY; // time to collision
		// iterate over neighbors
		int x = floor(m_pOwner.current_position.x / CUBE_SIZE);
		int y = floor(m_pOwner.current_position.z / CUBE_SIZE);

		int hash_index = x + MAP_DIMENSIONS * y;

		for (int i = -1; i <= MOORS_CONST; i++)
		{
			for (int j = -1; j <= MOORS_CONST; j++)
			{
				if ((x + i >= 0 && x + i < MAP_DIMENSIONS) && (y + j >= 0 && y + j < MAP_DIMENSIONS))
				{

					int hash_id = (x + i) + MAP_DIMENSIONS * (y + j);

					uint num_neighbours = SpatialIndexTableBuffer.Load(hash_id*UINT_SIZE);
					int index_agent_offset = (hash_id * NUM_AGENTS_PER_BLOCK);

					for (uint k = 0; k < num_neighbours; k++)
					{
						int index_agent = index_agent_offset + k;
						int agent_id_neighbour = SpatialAgentIdTableBuffer[index_agent];

						if (agent_id_neighbour != agent_Id){

							float ct_j; // time to collision with agent j
							float2 Vab;

							Agent other = agentList[agent_id_neighbour];

							//TODO: Test this explicitly
							Vab = 2.0f * vCand - m_pOwner.velocity_dir - other.velocity_dir;

							float time =
								computeTimeToCollison(
								m_pOwner.current_position.xz,
								Vab,
								other.current_position.xz,
								agent_radius + agent_radius,
								isCollison);

							if (isCollison) {
								ct_j = -ceil(time / _timeStep);
								ct_j -= absSq(vCand) / sqr(m_maxSpeedDefault);
							}
							else {
								ct_j = time;
							}


							if (ct_j < ct) {
								ct = ct_j;
								// pruning search if no better penalty can be obtained anymore for this velocity
								if (m_safetyFactorDefault / ct + dV >= min_penalty) {
									break;
								}
							}
						}
					}
				}
			}
		}
		

		float penalty = m_safetyFactorDefault / ct + dV;
		if (penalty < min_penalty) {
			min_penalty = penalty;
			return_final_velocity = vCand;
		}		
	}

	if (m_pOwner.agentId == 0){
		testRandomSample[251] = float2(m_pOwner.agentId, agent_Id);//return_final_velocity;
	}

	agentList[m_pOwner.agentId].seed = new_seed;

	return return_final_velocity;

}


[numthreads(NUM_THREAD_X, NUM_THREAD_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint gridId = ((NUM_THREAD_X * NUM_GRID_BLOCK_X) * DTid.y) + DTid.x;
	//int agentId[20];
	if (gridId == 0 || gridId == 1 || gridId == 2 || gridId == 3 || gridId == 4 || gridId == 5){

		float agent_half_width = AGENT_WIDTH / 2;
		float agent_radius = sqrt((sqr(agent_half_width)) + sqr(agent_half_width));

		agent_radius = agent_radius + agent_radius *0.3;

		uint offset = gridId * (MAP_DIMENSIONS * MAP_DIMENSIONS);
		// For testing take this i manually later make according DispatchId
		uint agent_Id = gridId;

		//float test = INFINITY;
		//bool test_inf = isinf(test);

		float3 m_current_position = float3(0.0, 0.0, 0.0);
		Agent agent = agentList[gridId];

		//agentId[2] = agent_Id;

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

				if (CUBE_SIZE  > temp_distance && interpol_id + 2 < agent.pathCount){
					interpol_id = interpol_id + 1;
					agentList[agent_Id].currentInterpolationId = interpol_id;
					interpol_B = pathList[offset + (interpol_id + 1)];
					coord_B = g_GridCenterListInput[interpol_B];
				}
				float2 temp = coord_B.xz - m_current_position.xz;
				float2 prefVelocity;
				float distSq2subgoal = absSq(temp);

				if (interpol_id + 2 > agent.pathCount && sqr(m_maxSpeedDefault * frameTime) > distSq2subgoal) {
					prefVelocity = temp / frameTime;
				}
				else{
					prefVelocity = (temp * m_prefSpeedDefault) / sqrt(distSq2subgoal);
				}

					/*int hash_index = floor(agent.current_position.x / CUBE_SIZE) +
					MAP_DIMENSIONS *(floor(agent.current_position.z / CUBE_SIZE));*/
				
					int x = floor(agent.current_position.x / CUBE_SIZE);
					int y = floor(agent.current_position.z / CUBE_SIZE);

					int hash_index = x + MAP_DIMENSIONS * y;

				bool isCollision = false;

				

				for (int i = -1; i <= MOORS_CONST; i++)
				{
					for (int j = -1; j <= MOORS_CONST; j++)
					{
						if ((x + i >= 0 && x + i < MAP_DIMENSIONS) && (y + j >= 0 && y + j < MAP_DIMENSIONS)){
						
							int hash_id = (x + i) + MAP_DIMENSIONS * (y + j);

							uint num_neighbours = SpatialIndexTableBuffer.Load(hash_id*UINT_SIZE);
							int index_agent_offset = (hash_id * NUM_AGENTS_PER_BLOCK);

							for (uint k = 0; k < num_neighbours; k++)
							{
								int index_agent = index_agent_offset + k;
								int agent_id_neighbour = SpatialAgentIdTableBuffer[index_agent];

								if (agent_id_neighbour != agent_Id){
									float dist = absSq(m_current_position.xz - agentList[agent_id_neighbour].current_position.xz);

									if (dist < sqr(agent_radius + agent_radius))// agent_radius)
									{
										isCollision = true;
									}
								}
							}

						}
					}
				}

			

				//(Agent m_pOwner, float agent_radius, int num_neighbours, int index_agent_offset, float2 prefVelocity, bool isCollison, float _timeStep)

				float2 return_final_velocity = simulateNewVelocity(agent_Id, agent, agent_radius,
					prefVelocity, isCollision, frameTime);

				float2 simulated_velocity;

				float dv = abs_vec(return_final_velocity - agent.velocity_dir);
				if (dv < m_maxAccelDefault * frameTime) {
					simulated_velocity = return_final_velocity;
				}
				else {
					simulated_velocity = (1 - (m_maxAccelDefault * frameTime / dv)) *  agent.velocity_dir +
						(m_maxAccelDefault * frameTime / dv) * return_final_velocity;
				}

				

					//m_current_position += normalize(temp) * agent.velocity * frameTime;

					m_current_position += float3(simulated_velocity.x, 0.0, simulated_velocity.y) *frameTime;

				agentList[agent_Id].current_position = m_current_position;
				agentList[agent_Id].velocity_dir = simulated_velocity;
				
				bufferOut[agent_Id] = float3(m_current_position.x + agent.randomFactor_X,
					coord_B.y, m_current_position.z + agent.randomFactor_Z);

		}
	}
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
