////////////////////////////////////////////////////////////////////////////////
// Filename: MultiAgentDrawClass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "MultiAgentDrawClass.h"
#include "MultiAgentDraw_CSCompiled.h"
#include "MultiAgentDraw_VSCompiled.h"
#include "MultiAgentDraw_PSCompiled.h"
#include "MultiAgentRender_GSCompiled.h"
#include "MultiAgentRender_PSCompiled.h"
#include "MultiAgentRender_VSCompiled.h"
#include "MultiAgentSpatialHash_CSCompiled.h"


#include <sstream>
MultiAgentDrawClass::MultiAgentDrawClass()
{
	m_vertexShader = 0;
	m_computeShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;
	m_computeshader_helper = new ComputeShaderHelperClass;
	m_ShaderUtility = new ShaderUtility;


}


MultiAgentDrawClass::MultiAgentDrawClass(const MultiAgentDrawClass& other)
{
}


MultiAgentDrawClass::~MultiAgentDrawClass()
{
}


bool MultiAgentDrawClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd);
	if (!result)
	{
		return false;
	}

	InitVertextBuffers(device, device_context);

	// SECTION:	Buffers
	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(XMFLOAT3), NUM_AGENTS, nullptr, &m_AgentPositionBuffer);

	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(XMFLOAT3), NUM_AGENTS, nullptr, &m_Buffer_AgentCurrentPosition);

	m_computeshader_helper->CreateVertexBuffer(device, sizeof(XMFLOAT3), NUM_AGENTS, nullptr, &m_AgentPositionDrawBuffer);

	/** Create a Raw Buffer for Spatial Hash Table*******************************/
	int empty_index[MAP_DIMENSIONS*MAP_DIMENSIONS];

	int empty_agentId[(MAP_DIMENSIONS*MAP_DIMENSIONS) * NUM_AGENTS_PER_BLOCK];

	std::fill_n(empty_index, MAP_DIMENSIONS*MAP_DIMENSIONS, 0);

	std::fill_n(empty_agentId, (MAP_DIMENSIONS*MAP_DIMENSIONS) * NUM_AGENTS_PER_BLOCK, 0);

	m_computeshader_helper->CreateRawBuffer(device, sizeof(int), MAP_DIMENSIONS*MAP_DIMENSIONS, &empty_index, &m_buffer_spatial_index_table_reset);

	m_computeshader_helper->CreateRawBuffer(device, sizeof(int), MAP_DIMENSIONS*MAP_DIMENSIONS, &empty_index, &m_buffer_spatial_index_table);

	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(int), (MAP_DIMENSIONS*MAP_DIMENSIONS) * NUM_AGENTS_PER_BLOCK,
		empty_agentId, &m_buffer_spatial_agent_id_table_reset);

	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(int), (MAP_DIMENSIONS*MAP_DIMENSIONS) * NUM_AGENTS_PER_BLOCK,
		empty_agentId, &m_buffer_spatial_agent_id_table);
	//********************************************************************************
	///////////////////////////////////////////////////////////////////////////////////

	// SECTION: Create UAV or Views for the Buffers
	m_computeshader_helper->CreateBufferUAV(device, m_AgentPositionBuffer, &m_view_AgentPosition_URV);

	m_computeshader_helper->CreateBufferUAV(device, m_Buffer_AgentCurrentPosition, &m_view_AgentCurrentPosition_URV);

	/** Create a Raw Buffer for Spatial Hash Table*******************************/
	m_computeshader_helper->CreateBufferUAV(device, m_buffer_spatial_index_table, &m_view_spatial_index_table_URV);

	m_computeshader_helper->CreateBufferUAV(device, m_buffer_spatial_agent_id_table, &m_view_spatial_agent_id_URV);
	///////////////////////////////////////////////////////////////////////////////////
	// Load Texture for Floor and other stuff
	m_FloorTextureSRV = m_ShaderUtility->CreateTextureFromFile(device, L"Textures/edited_floor.dds");
	m_CollisionWallSRV = m_ShaderUtility->CreateTextureFromFile(device, L"Textures/stone.dds");
	m_CubeTextureSRV = m_ShaderUtility->CreateTextureFromFile(device, L"Textures/cube.jpg");

	return true;
}

///////////////////////////////////////////////////////////////
///////TASKS//////////////////////////////////////////////////
// 1. Comple shader and create the shader instance
// 2. Define Input layout for the vertex shader, vertex buffer data.
// 3. Initiate the Constant shader variable buffer and texture buffers.
////////////////////////////////////////////////////////////////////
bool MultiAgentDrawClass::InitializeShader(ID3D11Device* device, HWND hwnd)
{
	HRESULT result;
	ID3D10Blob* errorMessage;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;

	// Compile and Create PixelShader Object
	result = device->CreateComputeShader(g_cshader, sizeof(g_cshader), nullptr, &m_computeShader);
	if (FAILED(result))
	{
		return false;
	}

	// Compile and Create PixelShader Object
	result = device->CreateComputeShader(g_cshader_spatial_hash, sizeof(g_cshader_spatial_hash), nullptr, &m_computeShader_spatial_hash);
	if (FAILED(result))
	{
		return false;
	}

	// Compile and Create VertexShader Object
	result = device->CreateVertexShader(g_vshader, sizeof(g_vshader), nullptr, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}


	// Compile and Create PixelShader Object
	result = device->CreatePixelShader(g_pshader, sizeof(g_pshader), nullptr, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}


	result = device->CreateGeometryShader(g_render_gshader, sizeof(g_render_gshader), nullptr, &m_render_geometryShader);
	if (FAILED(result))
	{
		return false;
	}

	// Compile and Create VertexShader Object
	result = device->CreateVertexShader(g_render_vshader, sizeof(g_render_vshader), nullptr, &m_render_vertexShader);
	if (FAILED(result))
	{
		return false;
	}


	// Compile and Create PixelShader Object
	result = device->CreatePixelShader(g_render_pshader, sizeof(g_render_pshader), nullptr, &m_render_pixelShader);
	if (FAILED(result))
	{
		return false;
	}


	result = createInputLayoutDesc(device);
	if (FAILED(result))
	{
		return false;
	}

	result = createConstantBuffer_TextureBuffer(device);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool MultiAgentDrawClass::createInputLayoutDesc(ID3D11Device* device)
{
	HRESULT result = true;

	ID3D10Blob* errorMessage;

	unsigned int numElements;

	const D3D11_INPUT_ELEMENT_DESC Basic32[3] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	// Get a count of the elements in the layout.
	numElements = sizeof(Basic32) / sizeof(Basic32[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(Basic32, numElements, g_vshader, sizeof(g_vshader), &m_layout);

	//////////////////////////////////////////////////////////////////////////
	// Layout for Render Agents 
	const D3D11_INPUT_ELEMENT_DESC renderAgents_DESC[1] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Get a count of the elements in the layout.
	numElements = sizeof(renderAgents_DESC) / sizeof(renderAgents_DESC[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(renderAgents_DESC, numElements, g_render_vshader, sizeof(g_render_vshader), &m_render_layout);


	return true;
}


bool MultiAgentDrawClass::createConstantBuffer_TextureBuffer(ID3D11Device* device)
{

	D3D11_BUFFER_DESC drawBufferDesc;
	HRESULT result = true;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	drawBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	drawBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	drawBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	drawBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	drawBufferDesc.MiscFlags = 0;
	drawBufferDesc.StructureByteStride = 0;

	drawBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&drawBufferDesc, NULL, &m_world_matrix_buffer);
	if (FAILED(result))
	{
		result = false;
	}



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

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool MultiAgentDrawClass::InitVertextBuffers(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
	MultiAgentDrawClass::InitFloorGeometryVertextBuffers(device, device_context);

	return true;
}


bool MultiAgentDrawClass::InitFloorGeometryVertextBuffers(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
	const int GRID_SIZE = 8;

	HRESULT result;
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 20.0f, GRID_SIZE + 1, GRID_SIZE + 1, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// Cache the index count of each object.
	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount =
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex = grid.Vertices[i].TexC;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex = sphere.Vertices[i].TexC;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex = cylinder.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	result = device->CreateBuffer(&vbd, &vinitData, &mShapesVB);
	if (FAILED(result))
	{
		return false;
	}
	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	result = device->CreateBuffer(&ibd, &iinitData, &mShapesIB);

	if (FAILED(result))
	{
		return false;
	}

	std::cout << "GridSize Original \n";
	////////////////////////////////////////////////////////////////////////////////////////
	// Create the texture from Center data for the Grid Cells
	for (int i = 0; i < GRID_SIZE *GRID_SIZE; i++){
		std::cout << " x_c::" << grid.Centers[i].x << " y_c::" << grid.Centers[i].y << " z_c::" << grid.Centers[i].z << "\n";
	}

	XMFLOAT3 tempArray[GRID_SIZE*GRID_SIZE];
	for (int i = 0; i < GRID_SIZE*GRID_SIZE; i++)
	{
		tempArray[i] = grid.Centers[i];
	}

	m_computeshader_helper->CreateStructuredBuffer(device, sizeof(XMFLOAT3), grid.Centers.size(), &tempArray, &m_Buffer_GridCenterData);
	m_computeshader_helper->CreateBufferSRV(device, m_Buffer_GridCenterData, &m_FloorCenterDataSRV);



	/////////////////////////////////////////////////////////////////////////////////////////
	return true;
}

bool MultiAgentDrawClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, float frameTime)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	//unsigned int bufferNumber;
	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_world_matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;
	dataPtr->gridScaling = XMMatrixScaling(8.0f, 8.0f, 1.0f); //TODO: Transform Floor
	dataPtr->frameTime = frameTime;
	//cout<< "CamPos||X:" <<camEyePos.x << "||Y:"<<camEyePos.y <<"||Z:"<<camEyePos.z<<"||\n";
	// Unlock the constant buffer.
	deviceContext->Unmap(m_world_matrix_buffer, 0);
	///////////////////////////////////////////////////////////////
	// Geometry Shader Constant Resources Set HERE
	deviceContext->VSSetConstantBuffers(0, 1, &m_world_matrix_buffer);
	deviceContext->CSSetConstantBuffers(0, 1, &m_world_matrix_buffer);
	//deviceContext->PSSetShaderResources(0, 1, &m_CubeMapSRV);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	/////////////////////////////////////////////////////////////////

	return true;
}

bool MultiAgentDrawClass::Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, float frameTime, float gameTime, XMFLOAT3 camEyePos,
	ID3D11UnorderedAccessView*  m_BufRenderAgentList_URV, ID3D11UnorderedAccessView*  m_BufRenderAgentPathList_URV)
{
	cout << "FireParticle:" << frameTime << "||" << gameTime << "||\n";
	D3DXMATRIX worldMatrix1 = *(new D3DXMATRIX);

	D3DXMatrixTranspose(&worldMatrix1, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	SetShaderParameters(deviceContext, worldMatrix1, viewMatrix, projectionMatrix, frameTime);

	// 0. interpolate the position of agents 
	RenderComputeSpatialHashShader(device, deviceContext, worldMatrix, viewMatrix, projectionMatrix,
		camEyePos, m_BufRenderAgentList_URV, m_BufRenderAgentPathList_URV);

	// 1. interpolate the position of agents 
	RenderComputeShader(device, deviceContext, worldMatrix, viewMatrix, projectionMatrix,
		camEyePos, m_BufRenderAgentList_URV, m_BufRenderAgentPathList_URV);

	// 2. Just render the agents with cubes using geometry shader
	RenderMultipleAgentShader(device, deviceContext, worldMatrix,
		viewMatrix, projectionMatrix, camEyePos);

	// 3. Render floor for agents
	RenderShader(device, deviceContext, worldMatrix,
		viewMatrix, projectionMatrix, camEyePos);

	return true;
}

void MultiAgentDrawClass::RenderComputeSpatialHashShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
	D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, XMFLOAT3 camEyePos,
	ID3D11UnorderedAccessView*  m_BufRenderAgentList_URV, ID3D11UnorderedAccessView*  m_BufRenderAgentPathList_URV)
{
	deviceContext->CopyResource(m_buffer_spatial_index_table, m_buffer_spatial_index_table_reset);

	deviceContext->CopyResource(m_buffer_spatial_agent_id_table, m_buffer_spatial_agent_id_table_reset);
	bool debug = false;
	//////////////////////////////////////////////////////////////////////////////////////////////
	// Dispatch ComputeShader
	if (debug){
		//ID3D11Buffer* debugbuf2 = m_computeshader_helper->CreateAndCopyToDebugBuf(device, deviceContext, m_buffer_spatial_index_table);
		//D3D11_MAPPED_SUBRESOURCE MappedResource2;
		//int *spatialIndexTableGPU;
		//deviceContext->Map(debugbuf2, 0, D3D11_MAP_READ, 0, &MappedResource2);

		//// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		//// This is also a common trick to debug CS programs.
		//spatialIndexTableGPU = (int*)MappedResource2.pData;

		//cout << " spatialIndexTableGPU before ******:\n";

		//int nodes2[65];
		//for (int i = 0; i < 64; i++)
		//{
		//	nodes2[i] = spatialIndexTableGPU[i];

		//	cout << "NodeId" << i << ":= " << spatialIndexTableGPU[i] << "\n";
		//}
		//deviceContext->Unmap(debugbuf2, 0);

		//debugbuf2->Release();
		//debugbuf2 = 0;

		//Log::Logger logger("MultiAgentDrawClass.cpp");
		//ID3D11Buffer* debugbuf2 = m_computeshader_helper->CreateAndCopyToDebugBuf(device, deviceContext, m_buffer_spatial_agent_id_table);
		//D3D11_MAPPED_SUBRESOURCE MappedResource2;
		//int *spatialIndexTableGPU;
		//deviceContext->Map(debugbuf2, 0, D3D11_MAP_READ, 0, &MappedResource2);

		//// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		//// This is also a common trick to debug CS programs.
		//spatialIndexTableGPU = (int*)MappedResource2.pData;

		//logger.debug() << " spatialIndexTableGPU AgentId's Before******:\n";

		//int nodes2[(MAP_DIMENSIONS*MAP_DIMENSIONS)*NUM_AGENTS_PER_BLOCK];
		//for (int i = 0; i < (MAP_DIMENSIONS*MAP_DIMENSIONS); i++)
		//{
		//	//nodes2[i] = spatialIndexTableGPU[i*NUM_AGENTS_PER_BLOCK];

		//	std::string result = "";

		//	for (int j = 0; j < (NUM_AGENTS_PER_BLOCK); j++)
		//	{
		//		int id = (i*NUM_AGENTS_PER_BLOCK ) + j;
		//		result = result + std::to_string((id)) + ":" + std::to_string(spatialIndexTableGPU[id]) + ",";
		//	}

		//	logger.debug() << "NodeId:" << i << ":= |" << result << "|";
		//}
		//deviceContext->Unmap(debugbuf2, 0);

		//debugbuf2->Release();
		//debugbuf2 = 0;
	}


	ID3D11UnorderedAccessView* aURViews[3] = { m_view_spatial_index_table_URV,
		m_view_spatial_agent_id_URV, m_BufRenderAgentList_URV
	};
	deviceContext->CSSetUnorderedAccessViews(0, 3, aURViews, nullptr);

	deviceContext->CSSetShader(m_computeShader_spatial_hash, nullptr, 0);

	deviceContext->Dispatch(2, 2, 1);

	deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* ppUAViewnullptr[3] = { nullptr, nullptr, nullptr };
	deviceContext->CSSetUnorderedAccessViews(0, 3, ppUAViewnullptr, nullptr);


	// Keep This commented for Spatial hash 
	//ID3D11Buffer* ppCBnullptr[1] = { nullptr };
	//deviceContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
	/////////////////////////////////////////////////////////////////////////////////////
	///// Check Output
	if (debug){

		ID3D11Buffer* debugbuf1 = m_computeshader_helper->CreateAndCopyToDebugBuf(device, deviceContext, m_buffer_spatial_index_table);
		D3D11_MAPPED_SUBRESOURCE MappedResource1;
		int *spatialIndexTableGPU;

		deviceContext->Map(debugbuf1, 0, D3D11_MAP_READ, 0, &MappedResource1);

		// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		// This is also a common trick to debug CS programs.
		spatialIndexTableGPU = (int*)MappedResource1.pData;

		//cout << " spatialIndexTableGPU After******:\n";

		/*	int nodes2[65];
			for (int i = 0; i < 64; i++)
			{
			nodes2[i] = spatialIndexTableGPU[i];

			cout << "NodeId" << i << ":= " << spatialIndexTableGPU[i] << "\n";
			}*/
		deviceContext->Unmap(debugbuf1, 0);

		debugbuf1->Release();
		debugbuf1 = 0;


		Log::Logger logger("MultiAgentDrawClass.cpp");
		ID3D11Buffer* debugbuf2 = m_computeshader_helper->CreateAndCopyToDebugBuf(device, deviceContext, m_buffer_spatial_agent_id_table);
		D3D11_MAPPED_SUBRESOURCE MappedResource2;
		int *spatialAgentIdTableGPU;
		deviceContext->Map(debugbuf2, 0, D3D11_MAP_READ, 0, &MappedResource2);

		// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		// This is also a common trick to debug CS programs.
		spatialAgentIdTableGPU = (int*)MappedResource2.pData;

		logger.debug() << " spatialIndexTableGPU AgentId's After******:\n";

		int nodes2[(MAP_DIMENSIONS*MAP_DIMENSIONS)*NUM_AGENTS_PER_BLOCK];
		for (int i = 0; i < (MAP_DIMENSIONS*MAP_DIMENSIONS); i++)
		{
			//nodes2[i] = spatialIndexTableGPU[i*NUM_AGENTS_PER_BLOCK];

			std::string result = "";

			for (int j = 0; j < (NUM_AGENTS_PER_BLOCK); j++)
			{
				int id = (i*NUM_AGENTS_PER_BLOCK) + j;
				result = result + std::to_string(spatialAgentIdTableGPU[id]) + " ,";
			}

			logger.debug() << "NodeId:" << i << "|count:" << std::to_string(spatialIndexTableGPU[i]) << ":= |" << result << "|";
		}
		deviceContext->Unmap(debugbuf2, 0);

		debugbuf2->Release();
		debugbuf2 = 0;
	}

}

void MultiAgentDrawClass::RenderComputeShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, XMFLOAT3 camEyePos,
	ID3D11UnorderedAccessView*  m_BufRenderAgentList_URV, ID3D11UnorderedAccessView*  m_BufRenderAgentPathList_URV)
{
	//////////////////////////////////////////////////////////////////////////////////////////////
	// Dispatch ComputeShader

	ID3D11ShaderResourceView* aRViews[1] = { m_FloorCenterDataSRV };

	ID3D11UnorderedAccessView* aURViews[6] = { m_view_AgentPosition_URV, m_BufRenderAgentList_URV,
		m_BufRenderAgentPathList_URV, m_view_AgentCurrentPosition_URV, m_view_spatial_index_table_URV,
		m_view_spatial_agent_id_URV };
	// Now render the prepared buffers with the shader.
	//deviceContext->CSSetConstantBuffers(0, 1, &m_BufConstantParameters);
	deviceContext->CSSetShaderResources(0, 1, aRViews);
	deviceContext->CSSetUnorderedAccessViews(0, 6, aURViews, nullptr);
	deviceContext->CSSetShader(m_computeShader, nullptr, 0);

	deviceContext->Dispatch(2, 2, 1);

	deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
	deviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);

	ID3D11ShaderResourceView* ppSRVnullptr[1] = { nullptr };
	deviceContext->CSSetShaderResources(0, 1, ppSRVnullptr);

	ID3D11Buffer* ppCBnullptr[1] = { nullptr };
	deviceContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
	/////////////////////////////////////////////////////////////////////////////////////
	///// Check Output

	//ID3D11Buffer* debugbuf2 = m_computeshader_helper->CreateAndCopyToDebugBuf(device, deviceContext, m_AgentPositionBuffer);
	//D3D11_MAPPED_SUBRESOURCE MappedResource2;
	//XMFLOAT3 *gridNodeListGPU;
	//deviceContext->Map(debugbuf2, 0, D3D11_MAP_READ, 0, &MappedResource2);

	//// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
	//// This is also a common trick to debug CS programs.
	//gridNodeListGPU = (XMFLOAT3*)MappedResource2.pData;

	//cout << " Center Map:\n";

	//XMFLOAT3 nodes2[65];
	//for (int i = 0; i < 64; i++)
	//{
	//	nodes2[i] = gridNodeListGPU[i];

	//	cout << i << ":X= " << gridNodeListGPU[i].x << "Y=" << gridNodeListGPU[i].y << " Z=" <<
	//		gridNodeListGPU[i].z << "\n";
	//}
	//deviceContext->Unmap(debugbuf2, 0);

	//debugbuf2->Release();
	//debugbuf2 = 0;


}
void MultiAgentDrawClass::RenderMultipleAgentShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, XMFLOAT3 camEyePos)
{
	bool result;
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Vertex, Pixel shader

	ID3D11Buffer* bufferArrayNull[1] = { 0 };

	D3DXMATRIX worldMatrix1 = *(new D3DXMATRIX);

	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;

	////////////////////////////////////////////////////////////////////////////////////
	// Set vertex Shader
	deviceContext->CopyResource(m_AgentPositionDrawBuffer, m_AgentPositionBuffer);

	deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	//deviceContext->P(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	deviceContext->IASetVertexBuffers(0, 1, &m_AgentPositionDrawBuffer, &stride, &offset);
	//deviceContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_render_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_render_vertexShader, NULL, 0);
	/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////
	// Set Geometry Shader
	deviceContext->GSSetConstantBuffers(0, 1, &m_world_matrix_buffer);
	deviceContext->GSSetShader(m_render_geometryShader, NULL, 0);
	////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////
	// Set vertex Shader
	ID3D11Buffer* bufferArray[1] = { 0 };
	ID3D11ShaderResourceView* aRTextureViews[1] = { m_CubeTextureSRV };
	deviceContext->SOSetTargets(1, bufferArray, 0);
	deviceContext->PSSetShaderResources(0, 1, aRTextureViews);
	deviceContext->PSSetShader(m_render_pixelShader, NULL, 0);
	// Set the sampler state in the pixel shader.
	//deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	////////////////////////////////////////////////////////////////////////////////////
	deviceContext->Draw(3200, 0);

	deviceContext->GSSetConstantBuffers(0, 1, bufferArray);
	deviceContext->GSSetShader(NULL, NULL, 0);
	return;
}


void MultiAgentDrawClass::RenderShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
	XMFLOAT3 camEyePos)
{
	bool result;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Vertex, Pixel shader

	ID3D11Buffer* bufferArrayNull[1] = { 0 };
	//XMMATRIX* worldMatrix1 = &XMMatrixTranslation(camEyePos.x, camEyePos.y, camEyePos.z);
	D3DXMATRIX worldMatrix1 = *(new D3DXMATRIX);


	UINT stride = sizeof(Basic32);
	UINT offset = 0;


	deviceContext->PSSetShaderResources(0, 1, &m_FloorTextureSRV);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	deviceContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);

	deviceContext->GSSetShader(NULL, NULL, 0);
	// deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	//deviceContext->SOSetTargets(1, bufferArrayNull, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	deviceContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

	return;
}



void MultiAgentDrawClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
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
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}





void MultiAgentDrawClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

void MultiAgentDrawClass::ShutdownShader()
{
	// Release the sampler state.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	// Release the vertex shader.
	if (m_computeShader)
	{
		m_computeShader->Release();
		m_computeShader = 0;
	}

	return;
}