////////////////////////////////////////////////////////////////////////////////
// Filename: MatrixMultiplyShaderClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "AStar_Type2_ShaderClass.h"
#include "AStar_Type2_ComputeShaderCompiled.h"

AStar_Type2_ShaderClass::AStar_Type2_ShaderClass()
{
	m_computeshader_helper = 0;
	m_computeShader = 0;

	m_BufConstantParameters = 0;

	m_Buffer_AgentList = 0;
	m_Buffer_SearchResult = 0;
	m_Buffer_OpenList = 0;
	m_Buffer_GridNodeListOut = 0;

	m_BufAgentList_SRV = 0;
	m_BufSearchResult_URV = 0;
	m_BufGridNodeListOut_URV = 0;
	m_BufOpenList_URV = 0;
	m_computeshader_helper = new ComputeShaderHelperClass;

}

AStar_Type2_ShaderClass::AStar_Type2_ShaderClass(const AStar_Type2_ShaderClass& other)
{
}

AStar_Type2_ShaderClass::~AStar_Type2_ShaderClass()
{
}

bool AStar_Type2_ShaderClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, deviceContext, hwnd, L"AStar_Type1_ComputeShader.hlsl");
	if (!result)
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////
///////TASKS//////////////////////////////////////////////////
// 1. Comple shader and create the shader instance
// 2. Define Input layout for the vertex shader, vertex buffer data.
// 3. Initiate the Constant shader variable buffer and texture buffers.
////////////////////////////////////////////////////////////////////
bool AStar_Type2_ShaderClass::InitializeShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, WCHAR* csFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* computeShaderBuffer;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	computeShaderBuffer = 0;

	// Compile the vertex shader code.
	//result = D3DX11CompileFromFile(csFilename, NULL, NULL, "main", "cs_5_0", D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL, 
	//	&computeShaderBuffer, &errorMessage, NULL);







	// Create the vertex shader from the buffer.
	//result = device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(),
	//	NULL, &m_computeShader);

	result = device->CreateComputeShader(g_csshader, sizeof(g_csshader), nullptr, &m_computeShader);
	if (FAILED(result))
	{
		return false;
	}

	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			//	OutputShaderErrorMessage(errorMessage, hwnd, csFilename);

			char* compileErrors;
			unsigned long bufferSize, i;
			ofstream fout;


			// Get a pointer to the error message text buffer.
			compileErrors = (char*)(errorMessage->GetBufferPointer());

			// Get the length of the message.
			bufferSize = errorMessage->GetBufferSize();

			// Open a file to write the error message to.
			fout.open("shader-error.txt");

			// Write out the error message.
			for (i = 0; i<bufferSize; i++)
			{
				fout << compileErrors[i];
			}

			// Close the file.
			fout.close();

			// Release the error message.
			errorMessage->Release();
			errorMessage = 0;
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, csFilename, L"Missing Shader File", MB_OK);
		}
		return false;
	}
	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.



	result = createConstantBuffer(device, deviceContext);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool AStar_Type2_ShaderClass::createConstantBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

	//for ( int i = 0; i < NUM_ELEMENTS; ++i ) 
	//{
	//	// g_vBuf0[i] = i;
	//	g_vBuf0[i].f = (float)i;

	//	g_vBuf1[i].f = (float)i;
	//}


	Agent agentList[NUM_AGENTS];

	Agent a1;

	int2 i;
	i.x1 = 0;
	i.y1 = 4;

	int2 i1;
	i1.x1 = 6;
	i1.y1 = 4;


	a1.sourceLoc = i;
	a1.targetLoc = i1;


	for (int i = 0; i < NUM_AGENTS; i++)
	{
		a1.id = i;
		agentList[i] = a1;
	}

	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(Agent), NUM_AGENTS, &agentList, &m_Buffer_AgentList);
	m_computeshader_helper->CreateBufferSRV(device, m_Buffer_AgentList, &m_BufAgentList_SRV);

	TextureUtility* m_TextureUtility = new TextureUtility;
	m_WorldMap_SRV = m_TextureUtility->CreateRandomTexture2DSRV_New(device, deviceContext);//,deviceContext);


	D3D11_SAMPLER_DESC samplerDesc;
	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT result;
	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;

}


bool AStar_Type2_ShaderClass::Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int vertexCount, int instanceCount,
	ID3D11Buffer* m_StreamOutBuffer, ID3D11ShaderResourceView* texture)
{
	bool result;
	UINT X = 2;
	UINT Y = 2;
	UINT Z = 1;

	// Set the shader parameters that it will use for rendering.
	//result = SetShaderParameters(deviceContext, texture);
	//if(!result)
	//{
	//	return false;
	//}



	AStarParameters parameters;
	parameters.MAP_DIMENSIONS = MAP_DIMENSIONS; // Like for 8x8 put 8 they are always semetric
	parameters.NUM_GRID_BLOCK_X = 23;//NUM_GRID_BLOCK_X;
	m_computeshader_helper->CreateConstantBuffer(device, sizeof(AStarParameters), 1, &parameters, &m_BufConstantParameters);
	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(int), NUM_OPENLIST_COUNT, nullptr, &m_Buffer_OpenList);
	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(int7), NUM_OPENLIST_COUNT, nullptr, &m_Buffer_GridNodeListOut);
	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(SearchResult), NUM_AGENTS, nullptr, &m_Buffer_SearchResult);

	m_computeshader_helper->CreateBufferUAV(device, m_Buffer_OpenList, &m_BufOpenList_URV);
	m_computeshader_helper->CreateBufferUAV(device, m_Buffer_GridNodeListOut, &m_BufGridNodeListOut_URV);
	m_computeshader_helper->CreateBufferUAV(device, m_Buffer_SearchResult, &m_BufSearchResult_URV);

	ID3D11ShaderResourceView* aRViews[2] = { m_BufAgentList_SRV, m_WorldMap_SRV };

	ID3D11UnorderedAccessView* aURViews[3] = { m_BufOpenList_URV, m_BufSearchResult_URV, m_BufGridNodeListOut_URV };
	// Now render the prepared buffers with the shader.
	deviceContext->CSSetConstantBuffers(0, 1, &m_BufConstantParameters);
	deviceContext->CSSetShaderResources(0, 2, aRViews);
	deviceContext->CSSetUnorderedAccessViews(0, 3, aURViews, nullptr);//3, aURViews, nullptr);
	deviceContext->CSSetShader(m_computeShader, nullptr, 0);
	//if ( pCBCS && pCSData )
	//{
	//    D3D11_MAPPED_SUBRESOURCE MappedResource;
	//    deviceContext->Map( pCBCS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	//    memcpy( MappedResource.pData, pCSData, dwNumDataBytes );
	//    deviceContext->Unmap( pCBCS, 0 );-
	//    ID3D11Buffer* ppCB[1] = { pCBCS };
	//    deviceContext->CSSetConstantBuffers( 0, 1, ppCB );
	//}

	//deviceContext->CSSetSamplers(0,1, &m_sampleState);
	//time_t start, end;
	//time(&start);
	deviceContext->Dispatch(DISPATCH_X, DISPATCH_Y, 1);
	//time(&end);
	//	double dif = difftime(end, start);
	//printf("Elasped time is %.2lf seconds.", dif);
	deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* ppUAViewnullptr[3] = { nullptr, nullptr, nullptr };
	deviceContext->CSSetUnorderedAccessViews(0, 3, ppUAViewnullptr, nullptr);

	ID3D11ShaderResourceView* ppSRVnullptr[1] = { nullptr };
	deviceContext->CSSetShaderResources(0, 1, ppSRVnullptr);

	ID3D11Buffer* ppCBnullptr[1] = { nullptr };
	deviceContext->CSSetConstantBuffers(0, 1, ppCBnullptr);

	/////////////////////////////////////////////////////////////////////
	//ID3D11Buffer* debugbuf1 = m_computeshader_helper->CreateAndCopyToDebugBuf(device, deviceContext, m_Buffer_OpenList);
	//D3D11_MAPPED_SUBRESOURCE MappedResource1;
	//int *p1;
	//deviceContext->Map(debugbuf1, 0, D3D11_MAP_READ, 0, &MappedResource1);

	//// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
	//// This is also a common trick to debug CS programs.
	//p1 = (int*)MappedResource1.pData;

	//int nodes1[NUM_OPENLIST_COUNT];
	//for (int i = 0; i < NUM_OPENLIST_COUNT; i++)

	//{
	//	nodes1[i] = p1[i];
	//}
	//deviceContext->Unmap(debugbuf1, 0);

	//debugbuf1->Release();
	//debugbuf1 = 0;
	///////////////////////////////////////////////////////////////////////////
	ID3D11Buffer* debugbuf2 = m_computeshader_helper->CreateAndCopyToDebugBuf(device, deviceContext, m_Buffer_GridNodeListOut);
	D3D11_MAPPED_SUBRESOURCE MappedResource2;
	int7 *gridNodeListGPU;
	deviceContext->Map(debugbuf2, 0, D3D11_MAP_READ, 0, &MappedResource2);

	// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
	 //This is also a common trick to debug CS programs.
	gridNodeListGPU = (int7*)MappedResource2.pData;

	int7 nodes2[64];
	for (int i = 0; i < 64; i++)
	{
		nodes2[i] = gridNodeListGPU[i];
		cout << "GridNodeListOut Content:" << i << ":F_COST: " << gridNodeListGPU[i].cost << "= H_COST:" << gridNodeListGPU[i].H_cost << "+ G_COST:" <<
			gridNodeListGPU[i].G_cost << " Parent:" << gridNodeListGPU[i].parentId << "\n";
	}
	deviceContext->Unmap(debugbuf2, 0);

	debugbuf2->Release();
	debugbuf2 = 0;
	/////////////////////////////////////////////////////////////////////////
	ID3D11Buffer* debugbuf3 = m_computeshader_helper->CreateAndCopyToDebugBuf(device, deviceContext, m_Buffer_SearchResult);
	D3D11_MAPPED_SUBRESOURCE MappedResource3;
	SearchResult *p3;
	deviceContext->Map(debugbuf3, 0, D3D11_MAP_READ, 0, &MappedResource3);

	// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
	// This is also a common trick to debug CS programs.
	p3 = (SearchResult*)MappedResource3.pData;

	SearchResult nodes3[64];
	for (int i = 0; i < 64; i++)
	{
		nodes3[i] = p3[i];

	}
	deviceContext->Unmap(debugbuf3, 0);

	debugbuf3->Release();
	debugbuf3 = 0;
	///////////////////////////////////////////////////////////////////

	////	 Verify that if Compute Shader has done right
	printf("Verifying against CPU result...");
	////////////////////////////////////////////////////////////////////
	// Extract the result of A* search from GPU

	int  *pathList;
	pathList = new (nothrow)int[NUM_OPENLIST_COUNT];          //size must be of type int 

	if (pathList == nullptr)
	{
		std::cout << "Error: memory could not be allocated";
	}

	SearchResult pathfindingResult;
	AgentRender agentRenderList[10];
	for (int i = 0; i < 10; i++)
	{
		pathfindingResult = p3[i];
	std:vector<int> pathVector;
		if (pathfindingResult.result == 1){
			// Generate the Path
			int sourceGridId = pathfindingResult.sourceGridId;
			int currentGridId = pathfindingResult.finalGridId; // Start pathbuilding with target node
			int offset = i * (MAP_DIMENSIONS * MAP_DIMENSIONS);
			int pathArray[64];
			int counter = 0;
			pathArray[counter] = currentGridId;
			pathList[offset + counter] = currentGridId;
			pathVector.push_back(currentGridId);
			counter++;

			while (sourceGridId != currentGridId){
				int  parentId = gridNodeListGPU[offset + currentGridId].parentId;
				//pathArray[counter] = parentId;
				pathVector.push_back(parentId);
				currentGridId = parentId;
				counter++;
			}
			int counter1 = 0;
			while (!pathVector.empty()){
				pathArray[counter1] = pathVector.back();
				pathList[offset + counter] = pathVector.back();
				pathVector.pop_back();
				counter1++;
			}

			AgentRender agent;
			agent.agentId = pathfindingResult.agentId;
			agent.pathCount = counter;
			agentRenderList[i] = agent;
		}
	}


	delete[] pathList;


	return true;
}

bool AStar_Type2_ShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture)
{
	return true;
}

void AStar_Type2_ShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

void AStar_Type2_ShaderClass::ShutdownShader()
{
	// Release the sampler state.
	if (m_Buffer_AgentList)
	{
		m_Buffer_AgentList->Release();
		m_Buffer_AgentList = 0;
	}
	if (m_Buffer_GridNodeListOut)
	{
		m_Buffer_GridNodeListOut->Release();
		m_Buffer_GridNodeListOut = 0;
	}

	if (m_Buffer_OpenList)
	{
		m_Buffer_OpenList->Release();
		m_Buffer_OpenList = 0;
	}

	if (m_Buffer_SearchResult)
	{
		m_Buffer_SearchResult->Release();
		m_Buffer_SearchResult = 0;
	}

	// Release the layout.
	if (m_computeShader)
	{
		m_computeShader->Release();
		m_computeShader = 0;
	}

	// Release the pixel shader.
	if (m_BufAgentList_SRV)
	{
		m_BufAgentList_SRV->Release();
		m_BufAgentList_SRV = 0;
	}

	// Release the vertex shader.
	if (m_BufSearchResult_URV)
	{
		m_BufSearchResult_URV->Release();
		m_BufSearchResult_URV = 0;
	}


	if (m_BufGridNodeListOut_URV)
	{
		m_BufGridNodeListOut_URV->Release();
		m_BufGridNodeListOut_URV = 0;
	}
	if (m_BufOpenList_URV)
	{
		m_BufOpenList_URV->Release();
		m_BufOpenList_URV = 0;
	}
	return;
}