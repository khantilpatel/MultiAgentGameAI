#define NUM_THREAD_X 1
#define NUM_THREAD_Y 1

cbuffer AStarParameters : register(b0){
	int NUM_GRID_BLOCK_X;
	int MAP_DIMENSIONS;
};

struct BufType
{
	uint f;
};

struct SubType
{
	uint f1;
};

struct ParentType
{
	SubType sub[40];
};

struct Node
{
	uint2 coords;
	uint cost;
	uint id;
};

struct Agent
{
	uint id;
	uint2 sourceLoc;
	uint2 targetLoc;
};

struct SearchResult
{
	uint agentId;
	uint finalCost;
	uint sourceGridId;
	uint finalGridId;
	uint result;
};

struct uint7{
	uint x;
	uint y;
	uint parentId;
	uint status;
	uint cost;
	uint G_cost;
	uint pqId;
	uint H_cost;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};


//static const uint _GRID_RESOLUTION_X_AXIS = 8;



// Input List with Agents data from CPU
StructuredBuffer<Agent> gAgentListInput : register(t0);

// World map 2D Texture from CPU
Texture2D<uint4> m_map : register(t2);

// Temp OpenList for A* executed by each Agent 
// x = ID of the Node
// y = Cost of the node
// z = G cost
RWStructuredBuffer<uint> gOpenListOut: register(u0);

// Temp Grid List to maintain the status and target pointers
// x, y = 2D location on grid
// z = ID of the parent pointer node
// w = Status of Node Unread = 0; Open = 1; Closed = 2; UnWalkable=3;
RWStructuredBuffer<uint7> gGridNodeListOut: register(u2);
// Final Result of A* for each Agent
RWStructuredBuffer<SearchResult> gBufferOut : register(u1);
//RWStructuredBuffer<ParentType> BufferOutClone : register(u1);
//



uint getGridIntegerCoordinate(uint x, uint y)
{
	uint gridId = MAP_DIMENSIONS * y + x;
	return gridId;
}

void insertPQ(uint offset, uint x, uint y, uint current_cost, uint H_cost, uint G_cost, uint parentId, uint status){


	// 1.First get the Grid ID of the new node
	uint gridId = getGridIntegerCoordinate(x, y);

	// 3. Insert the Grid ID into the PQ according to its current_cost. 
	uint currentSize = gOpenListOut[offset + 0];

	uint i = currentSize + 1;

	uint parent_cost = gGridNodeListOut[offset + gOpenListOut[offset + (i / 2)]].cost;
	[allow_uav_condition]
	while (i > 1 && parent_cost > current_cost)
	{
		gOpenListOut[offset + i] = gOpenListOut[offset + (i / 2)];
		gGridNodeListOut[offset + gOpenListOut[offset + i]].pqId = i;

		i = i / 2;
		parent_cost = gGridNodeListOut[offset + gOpenListOut[offset + (i/2)]].cost;
	}

	// 2. Insert into GridNodeList with cost
	uint7 GridNode;
	GridNode.x = x;
	GridNode.y = y;
	GridNode.cost = current_cost;
	GridNode.parentId = parentId;
	GridNode.status = status;
	GridNode.G_cost = G_cost;
	GridNode.pqId = i;
	GridNode.H_cost = H_cost;
	gGridNodeListOut[offset + gridId] = GridNode;

	//Insert into PQ;
	gOpenListOut[offset + i] = gridId;
	gOpenListOut[offset + 0] = gOpenListOut[offset + 0] + 1;
	//gOpenListOut[i] = node;

}

void updatePQ(uint offset, uint pqId)
{

	// 1.First get the Grid ID of the new node

	// 3. Insert the Grid ID into the PQ according to its current_cost. 
	//uint currentSize = gOpenListOut[0];

	uint i = pqId; // currentSize + 1;

	uint orignal_value = gOpenListOut[offset + i];
	uint current_cost = gGridNodeListOut[offset + orignal_value].cost;
	uint parent_cost = gGridNodeListOut[offset + gOpenListOut[offset + (i / 2)]].cost;

	[allow_uav_condition]
	while (i > 1 && parent_cost > current_cost)
	{
		gOpenListOut[offset + i] = gOpenListOut[offset + (i / 2)];
		gGridNodeListOut[offset + gOpenListOut[offset + i]].pqId = i;

		i = i / 2;
		parent_cost = gGridNodeListOut[offset + gOpenListOut[offset + (i / 2)]].cost;
	}

	gOpenListOut[offset + i] = orignal_value;
	gGridNodeListOut[offset + gOpenListOut[offset + i]].pqId = i;

}

uint removePQ(uint offset)
{

	uint currentSize = gOpenListOut[offset + 0];

	uint newTemp = 0;


	uint nodeReturn = newTemp;

	if (currentSize >= 1)
	{
		nodeReturn = gOpenListOut[offset + 1];

		gOpenListOut[offset + 1] = gOpenListOut[offset + currentSize];

		gGridNodeListOut[offset + gOpenListOut[offset + 1]].pqId = 1;

		gOpenListOut[offset + currentSize] = newTemp;

		currentSize = currentSize - 1;

		gOpenListOut[offset + 0] = currentSize;

		uint i = 1;

		bool flag = false;

		if (currentSize >= 1)
		{
			[allow_uav_condition]
			while (true)
			{
				uint rightChild = (i * 2) + 1;
				uint leftChild = i * 2;
				uint replaceId = 1;
				uint currentNodeCost;
				uint replaceableChildCost = 0;
				if (rightChild >= currentSize)
				{
					if (leftChild >= currentSize)
					{
						break;
					}
					else
						replaceId = leftChild;
					replaceableChildCost = gGridNodeListOut[offset + gOpenListOut[offset + replaceId]].cost;;
				}
				else
				{
					uint leftChildCost = gGridNodeListOut[offset + gOpenListOut[offset + leftChild]].cost;
					uint rightChildCost = gGridNodeListOut[offset + gOpenListOut[offset + rightChild]].cost;

					if (leftChildCost <= rightChildCost)
					{
						replaceId = leftChild;
						replaceableChildCost = leftChildCost;
					}
					else
					{
						replaceId = rightChild;
						replaceableChildCost = rightChildCost;
					}
				}
				currentNodeCost = gGridNodeListOut[offset + gOpenListOut[offset + i]].cost;
				if (currentNodeCost > replaceableChildCost)
				{
					uint temp;

					temp = gOpenListOut[offset + replaceId];
					gOpenListOut[offset + replaceId] = gOpenListOut[offset + i];
					gOpenListOut[offset + i] = temp;

					gGridNodeListOut[offset + gOpenListOut[offset + replaceId]].pqId = replaceId;
					gGridNodeListOut[offset + gOpenListOut[i]].pqId = i;

					i = replaceId;
				}
				else{
					break;
				}

			}
		}
	}


	return nodeReturn;

}


groupshared int shared_data[30];
//void InsertIntoGridNodeList(uint x, uint y, uint parentId, uint status)
//{
//	uint gridId = getGridIntegerCoordinate(x, y);
//	gGridNodeListOut[gridId] = uint4(x, y, parentId, status);
//}

void addToOpenList(uint offset, uint2 thisNode, uint2 targetNode, uint parent_G_Cost,
	uint G_COST, uint outputSlot, uint parentId)
{
	shared_data[outputSlot] = -1;
	const uint _COLLISION_NODE = 1;
	const uint _STATUS_UNREAD = 0;
	const uint _NODE_OPEN = 1;
	// 1. check if status is _STATUS_UNREAD
	//if(m_map[thisNode].z != _COLLISION_NODE)
	//{
	uint thisGridNodeID = getGridIntegerCoordinate(thisNode.x, thisNode.y);
	uint7 thisGridNode = gGridNodeListOut[offset + thisGridNodeID];

	// 2. Check if not collision node on texture 2d
	if (thisGridNode.status == _STATUS_UNREAD)
	{
		// 3. Calculate G cost (parent_G + current)
		uint current_G_cost = parent_G_Cost + G_COST;

		int x1 = int(thisNode.x);
		int x2 = int(targetNode.x);

		int y1 = int(thisNode.y);
		int y2 = int(targetNode.y);
		// 4. calculate H cost
		int x_temp = abs(x1 - x2);
		int y_temp = abs(y1 - y2); //int y_temp = abs(asfloat(thisNode.y) - asfloat(targetNode.y));

		int temp = x_temp + y_temp;
		uint H = uint(temp);
		uint current_cost = current_G_cost + H * 10; // F = G + H
		
		//shared_data[outputSlot] = 1; // Result
		//shared_data[outputSlot + 1] = current_cost; // F cost
		//shared_data[outputSlot + 2] = current_G_cost; // G cost
		
		
		insertPQ(offset, thisNode.x, thisNode.y, current_cost, H*10, current_G_cost, parentId, _NODE_OPEN);
	
		// 5. insert into PQ
		// 6. set current node ID as parent
		// 7. set node status open;	
		// 8. Insert into GridNodeList
		//insertPQ(offset, thisNode.x, thisNode.y, current_cost, current_G_cost, parentGridId, _NODE_OPEN);

	}
	else if (thisGridNode.status == _NODE_OPEN)
	{
		uint current_G_cost = parent_G_Cost + G_COST;

		// 5. insert into PQ
		//insertPQ(pqNode, );
		if (current_G_cost < thisGridNode.G_cost)
		{
			int x1 = int(thisNode.x);
			int x2 = int(targetNode.x);
			int y1 = int(thisNode.y);
			int y2 = int(targetNode.y);
			// 4. calculate H cost
			int x_temp = abs(x1 - x2);
			int y_temp = abs(y1 - y2); //int y_temp = abs(asfloat(thisNode.y) - asfloat(targetNode.y));

			int temp = x_temp + y_temp;
			uint H = uint(temp);
			uint F = current_G_cost + H * 10; // F = G + H
			gGridNodeListOut[offset + thisGridNodeID].cost = F;

			gGridNodeListOut[offset + thisGridNodeID].G_cost = current_G_cost;
			gGridNodeListOut[offset + thisGridNodeID].H_cost = H * 10;

			gGridNodeListOut[offset + thisGridNodeID].parentId = parentId;


			uint pqId = thisGridNode.pqId;


			updatePQ(offset, pqId);

			//shared_data[outputSlot] = 2; // result 
			//shared_data[outputSlot + 1] = pqId;
			//shared_data[outputSlot + 2] = current_G_cost; // G cost
			//updatePQ(offset, pqId);
		}
	}
}

//void addToOpenList_Final (uint branchResult, uint x, uint y, uint SHARED_OFFSET, uint offset, uint parentGridId)
//{
//	//if (x == 8 && y == 7){
//	//	SearchResult pathfindingResult;
//	//	pathfindingResult.agentId = x;//agent.id;
//	//	pathfindingResult.finalCost = y;//pqCurrentNode.y;
//	//	pathfindingResult.sourceGridId = parentGridId;
//	//	pathfindingResult.finalGridId = 0;
//	//	pathfindingResult.result = SHARED_OFFSET;
//	//	gBufferOut[0] = pathfindingResult;
//	//}
//	
//
//	if (1 == branchResult){
//		uint F = shared_data[SHARED_OFFSET + 1];
//		uint G = shared_data[SHARED_OFFSET + 2];
//		insertPQ(offset, x, y, F, F-G , G, parentGridId, 1);
//	}
//	else if (2 == branchResult){
//		uint pqId = shared_data[SHARED_OFFSET + 1];
//		updatePQ(offset, pqId);
//	}
//}

[numthreads(8, 1, 1)] //NUM_THREAD_X
void main(uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex)
{
	///////////////////////////////////////////////////////////////
	///Thread Indexing testing
	//uint gridIdTest = (64 * Gid.y) + Gid.x;
	//SearchResult pathfindingResult;
	//pathfindingResult.agentId = Gid.x;//agent.id;
	//pathfindingResult.finalCost = Gid.y;//pqCurrentNode.y;
	//pathfindingResult.sourceGridId = 0;
	//pathfindingResult.finalGridId = 0;
	//pathfindingResult.result = 1;

	//gBufferOut[gridIdTest] = pathfindingResult;
	/////////////////////////////////////////////////////////////////
	Agent agent;
	uint gridId = 0; //= ((NUM_THREAD_X * NUM_GRID_BLOCK_X) * DTid.y) + DTid.x;
	uint offset = 0; // = gridId * (MAP_DIMENSIONS * MAP_DIMENSIONS);
	uint threadId = 0;//
	uint result = 0; // Final search result

	const int _NONE_UNREAD = 0;
	const int _NODE_OPEN = 1;
	const int _NODE_CLOSED = 2;
	const int _WALKABLE_NODE = 0;
	const int _COLLISION_NODE = 1;
	bool targetFound = false;
	int loopCounter = 0;
	uint4 currentGridNode = 0;
	uint targetNodeGridId = 0;
	uint sourceGridId = 0;
	uint m_GridId = 0;

	uint _RIGHT = 6;
	uint _B_RIGHT = 9;
	uint _BOTTOM = 12;
	uint _B_LEFT = 15;
	uint _LEFT = 18;
	uint _T_LEFT = 21;
	uint _TOP = 24;
	uint _T_RIGHT = 27;

	if (GI == 0)
	{
		//gridId = ((NUM_THREAD_X * NUM_GRID_BLOCK_X) * Gid.y) + Gid.x;
		gridId = (64 * Gid.y) + Gid.x;
		offset = gridId * (MAP_DIMENSIONS * MAP_DIMENSIONS);
		threadId = gridId;
		uint7 node;
		node.x = 4;
		node.y = 2;
		node.cost = 0;
		node.G_cost = 0;
		node.parentId = 0;
		node.status = 3;
		node.pqId = 0;
		node.H_cost = 0;
		gGridNodeListOut[offset + 20] = node;

		node.x = 4;
		node.y = 3;
		node.cost = 0;
		node.G_cost = 0;
		node.status = 3;
		gGridNodeListOut[offset + 28] = node;

		node.x = 4;
		node.y = 4;
		node.cost = 0;
		node.G_cost = 0;
		node.status = 3;
		gGridNodeListOut[offset + 36] = node;

		node.x = 4;
		node.y = 5;
		node.cost = 0;
		node.G_cost = 0;
		node.status = 3;
		gGridNodeListOut[offset + 44] = node;

		node.x = 4;
		node.y = 6;
		node.cost = 0;
		node.G_cost = 0;
		node.status = 3;
		gGridNodeListOut[offset + 52] = node;

		gOpenListOut[offset + 0] = 0;

		agent = gAgentListInput[threadId];

		// Get Source node to start the search with
		uint2 nodeFirstLoc;
		nodeFirstLoc = agent.sourceLoc;

		// Get Target node to check in the while loop if path found
		uint2 targetNode;
		targetNode = agent.targetLoc;
		targetNodeGridId = getGridIntegerCoordinate(targetNode.x, targetNode.y);
		sourceGridId = getGridIntegerCoordinate(nodeFirstLoc.x, nodeFirstLoc.y);
		// Insert the first node into OpenList;
		// &
		// Insert into GridNodeList
		insertPQ(offset, nodeFirstLoc.x, nodeFirstLoc.y, 10, 0 , 0, 0, _NODE_OPEN);

		shared_data[0] = targetNode.x;
		shared_data[1] = targetNode.y;
		shared_data[2] = offset;
	}
	
	if (0 <= GI && 7 >= GI){
		[loop]
		for (uint i = 0; i <= (MAP_DIMENSIONS * MAP_DIMENSIONS); i++) //(MAP_DIMENSIONS * MAP_DIMENSIONS)
		{
			//gBufferOut[threadId].agentId = i;
			uint7 currentNode;
			if (GI == 0 && targetFound != true)
			{
				m_GridId = removePQ(offset);
				gGridNodeListOut[offset + m_GridId].status = _NODE_CLOSED;
				currentNode = gGridNodeListOut[offset + m_GridId];
				//GroupMemoryBarrierWithGroupSync();
				// terminate if no path found or pathfound
				if (currentNode.cost == 0)
				{
					result = 0;
					targetFound = true;

					SearchResult pathfindingResult;
					pathfindingResult.agentId = agent.id;//agent.id;
					pathfindingResult.finalCost = currentNode.cost;//pqCurrentNode.y;
					pathfindingResult.sourceGridId = sourceGridId;
					pathfindingResult.finalGridId = m_GridId;
					pathfindingResult.result = result;

					gBufferOut[threadId] = pathfindingResult;
				}
				else if (m_GridId == targetNodeGridId){
					result = 1;
					targetFound = true;
					//i = 64;
					SearchResult pathfindingResult;
					pathfindingResult.agentId = loopCounter;//agent.id;//agent.id;
					pathfindingResult.finalCost = currentNode.cost;//pqCurrentNode.y;
					pathfindingResult.sourceGridId = sourceGridId;
					pathfindingResult.finalGridId = m_GridId;
					pathfindingResult.result = result;

					gBufferOut[threadId] = pathfindingResult;
				}
				else{
					loopCounter = loopCounter + 1;
					//currentGridNode = gGridNodeListOut[pqTopNode.x];		
					float tempx = float(currentNode.x);
					float tempy = float(currentNode.y);
					shared_data[3] = tempx;
					shared_data[4] = tempy;
					shared_data[5] = currentNode.G_cost;
					shared_data[6] = m_GridId;
				}

				//if (loopCounter==42) //8
				//	break;
				//}


			}

			///////////////////////////////////////////////////////////////////////
			GroupMemoryBarrierWithGroupSync();
			///////////////////////////////////////////////////////////////////////
			if (targetFound != true){
				uint2 targetNode = uint2(shared_data[0], shared_data[1]);
					uint offset = shared_data[2];
				float tempx = float(shared_data[3]);
				float tempy = float(shared_data[4]);
				uint parent_G_Cost = shared_data[5];
				m_GridId = shared_data[6];
				///// RIGHT //////////////////////////////////////////////////////////////////////////////////
				if (GI == 1)
				{
					if (MAP_DIMENSIONS > tempx + 1) // To the immidiate right
					{
						uint outputSlot = _RIGHT;
						uint G_COST = 10;
						uint2 thisNode = uint2(tempx + 1, tempy);
							addToOpenList(offset, thisNode, targetNode, parent_G_Cost, G_COST, outputSlot, m_GridId);

					}
				}

				////// BOTTOM-RIGHT ///////////////////////////////////////////////////////////////////////////
				if (GI == 2)
				{
					if (MAP_DIMENSIONS > (tempx + 1) && 0 <= (tempy - 1)) // To the immidiate right
					{
						uint outputSlot = _B_RIGHT;
						uint G_COST = 12;
						uint2 thisNode = uint2(tempx + 1, tempy - 1);
							addToOpenList(offset, thisNode, targetNode, parent_G_Cost, G_COST, outputSlot, m_GridId);
					}
				}

				////// BOTTOM ///////////////////////////////////////////////////////////////////////////////
				if (GI == 3)
				{

					if (0 <= (tempy - 1)) // To the immidiate right
					{
						uint outputSlot = _BOTTOM;
						uint G_COST = 10;
						uint2 thisNode = uint2(tempx, tempy - 1);
							addToOpenList(offset, thisNode, targetNode, parent_G_Cost, G_COST, outputSlot, m_GridId);
					}
				}

				////// _B_LEFT ////////////////////////////////////////////////////////////////////////////////
				if (GI == 4)
				{
					if (0 <= (tempx - 1) && 0 <= (tempy - 1)) // To the immidiate right
					{
						uint outputSlot = _B_LEFT;
						uint G_COST = 12;
						uint2 thisNode = uint2(tempx - 1, tempy - 1);
							addToOpenList(offset, thisNode, targetNode, parent_G_Cost, G_COST, outputSlot, m_GridId);
					}
				}

				///// _LEFT ///////////////////////////////////////////////////////////////////////////////////	
				if (GI == 5)
				{
					if (0 <= (tempx - 1))// To the immidiate right
					{
						uint outputSlot = _LEFT;
						uint G_COST = 10;
						uint2 thisNode = uint2(tempx - 1, tempy);
							addToOpenList(offset, thisNode, targetNode, parent_G_Cost, G_COST, outputSlot, m_GridId);
					}
				}

				///// _T_LEFT//////////////////////////////////////////////////////////////////////////////
				if (GI == 6)
				{
					if (0 <= (tempx - 1) && MAP_DIMENSIONS > (tempy + 1))// To the immidiate right
					{
						uint outputSlot = _T_LEFT;
						uint G_COST = 12;
						uint2 thisNode = uint2(tempx - 1, tempy + 1);
							addToOpenList(offset, thisNode, targetNode, parent_G_Cost, G_COST, outputSlot, m_GridId);
					}
				}

				///// _TOP //////////////////////////////////////////////////////////////////////////////
				if (GI == 7)
				{
					if (MAP_DIMENSIONS > (tempy + 1))// To the immidiate right
					{
						uint outputSlot = _TOP;
						uint G_COST = 10;
						uint2 thisNode = uint2(tempx, tempy + 1);
							addToOpenList(offset, thisNode, targetNode, parent_G_Cost, G_COST, outputSlot, m_GridId);
					}
				}

				///// _TOP_RIGHT //////////////////////////////////////////////////////////////////////////////
				if (GI == 0)
				{
					if (MAP_DIMENSIONS > (tempx + 1) && MAP_DIMENSIONS > (tempy + 1)) // To the immidiate right
					{
						uint outputSlot = _T_RIGHT;
						uint G_COST = 12;
						uint2 thisNode = uint2(tempx + 1, tempy + 1);
							addToOpenList(offset, thisNode, targetNode, parent_G_Cost, G_COST, outputSlot, m_GridId);
					}
				}
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////
		//GroupMemoryBarrierWithGroupSync();
		/////////////////////////////////////////////////////////////////////////////////////////////////
		// RIGHT
		//if (GI == 0 && targetFound != true){
		//	uint branchResult = shared_data[_RIGHT];
		//	if (1 == branchResult || 2 == branchResult)
		//	{
		//		shared_data[_RIGHT] = 0;
		//		addToOpenList_Final(branchResult, currentNode.x + 1, currentNode.y, _RIGHT, offset, m_GridId);
		//	}
		//	 branchResult = shared_data[_B_RIGHT];
		//	if (1 == branchResult || 2 == branchResult)
		//	{
		//		shared_data[_B_RIGHT] = 0;
		//		addToOpenList_Final(branchResult, currentNode.x + 1, currentNode.y - 1, _B_RIGHT, offset, m_GridId);
		//	}

		//	 branchResult = shared_data[_BOTTOM];
		//	if (1 == branchResult || 2 == branchResult)
		//	{
		//		shared_data[_BOTTOM] = 0;
		//		addToOpenList_Final(branchResult, currentNode.x, currentNode.y - 1, _BOTTOM, offset, m_GridId);
		//	}

		//	 branchResult = shared_data[_B_LEFT];
		//	if (1 == branchResult || 2 == branchResult)
		//	{
		//		shared_data[_B_LEFT] = 0;
		//		addToOpenList_Final(branchResult, currentNode.x - 1, currentNode.y - 1, _B_LEFT, offset, m_GridId);
		//	}

		//	 branchResult = shared_data[_LEFT];
		//	if (1 == branchResult || 2 == branchResult)
		//	{
		//		shared_data[_LEFT] = 0;
		//		addToOpenList_Final(branchResult, currentNode.x - 1, currentNode.y, _LEFT, offset, m_GridId);
		//	}


		//	 branchResult = shared_data[_T_LEFT];
		//	if (1 == branchResult || 2 == branchResult)
		//	{
		//		shared_data[_T_LEFT] = 0;
		//		addToOpenList_Final(branchResult, currentNode.x - 1, currentNode.y + 1, _T_LEFT, offset, m_GridId);
		//	}

		//	 branchResult = shared_data[_TOP];
		//	if (1 == branchResult || 2 == branchResult)
		//	{
		//		shared_data[_TOP] = 0;
		//		addToOpenList_Final(branchResult, currentNode.x, currentNode.y + 1, _TOP, offset, m_GridId);
		//	}

		//	 branchResult = shared_data[_T_RIGHT];
		//	if (1 == branchResult || 2 == branchResult)
		//	{
		//		shared_data[_T_RIGHT] = 0;
		//		addToOpenList_Final(branchResult, currentNode.x + 1, currentNode.y + 1, _T_RIGHT, offset, m_GridId);
		//	}
		//	//i = 64;
		//}
		//GroupMemoryBarrierWithGroupSync();
		// BOTTOM-RIGHT
		// BOTTOM
		// BOTTOM-LEFT
		// LEFT
		// TOP-LEFT
		// TOP
		// TOP-RIGHT

	}

}

