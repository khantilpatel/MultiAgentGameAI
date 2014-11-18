#include "RainParticleSystem.h"
#include <iostream>

const LPCWSTR STR_CLASSNAME = L"RainParticleSystem";
RainParticleSystem::RainParticleSystem()
{
	m_initBuffer = 0;
	m_DrawBuffer = 0;
	m_StreamOutBuffer = 0;
	m_StagingStreamOutBuffer = 0;
	m_SO_constant_buffer= 0 ;
	// Shader Variables 
	m_vertexShader_StreamOut = 0;
	m_pixelShader_StreamOut = 0;
	m_geometryShader_StreamOut = 0;

	m_vertexShader_Draw = 0;
	m_pixelShader_Draw = 0;
	m_geometryShader_Draw = 0;

	m_layout_streamOut = 0;
	m_layout_Draw = 0;

	//m_MaxParticles = 0;

	m_world_matrix_buffer = 0;
	m_sampleState_Shader = 0;

	m_ShaderUtility = new ShaderUtility;

	m_EyePosW  = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Emit_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_EmitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);
}


RainParticleSystem::~RainParticleSystem()
{

}


RainParticleSystem::RainParticleSystem(const RainParticleSystem& other){



}


bool RainParticleSystem::Render(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, 
									  D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
									  float frameTime, float gameTime, XMFLOAT3 camEyePos )
{
	HRESULT result;
	D3DXMATRIX* ViewProj = new D3DXMATRIX;
	cout<< "FireParticle:" << frameTime << "||" << gameTime << "||\n";

	//D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	//D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	//D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);


	D3DXMatrixMultiply(ViewProj, &viewMatrix, &projectionMatrix);

	D3DXMatrixTranspose(ViewProj, ViewProj);

	setEmit_Position(camEyePos);
		///////// STREAM_OUT/////////
	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters_StreamOut(deviceContext, frameTime,gameTime,camEyePos,ViewProj);
	if(!result)
	{
		//return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader_StreamOut(deviceContext);


	result = SetShaderParameters_Draw(deviceContext, frameTime,gameTime,camEyePos,ViewProj);
	if(!result)
	{
		return false;
	}

	//RenderShader_Draw(deviceContext);
	return  true;
}



///////////////////////////////////////////////////////////////////////////////
// Binds Shader Parameters at runtime every frame:
// 1. ParticleParametersBufferType.
// 2. Random function Texture.
// 3. Sampler for the  texture sampling function.
bool RainParticleSystem::SetShaderParameters_StreamOut(ID3D11DeviceContext* deviceContext,
															 float timeStep, float gameTime,
																 XMFLOAT3 camEyePos, D3DXMATRIX* ViewProj )
					 
{

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE so_mappedResource;	
	SOBufferType* so_dataPtr;


	
	////////////////////////////////////////////////////////////
	// Copy the values from CPU to Buffer for later use in GPU
	////////////////////////////////////////////////////////////

	
//	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_SO_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &so_mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	so_dataPtr = (SOBufferType*)so_mappedResource.pData;
	//SOBufferType* data = new SOBufferType;
	so_dataPtr->gEmitPosition = camEyePos;// m_Emit_Position;
	so_dataPtr->gFrameTime = timeStep;
	so_dataPtr->gGameTime = gameTime;
	so_dataPtr->gEyePosition = m_Emit_Position;

//	memcpy(so_mappedResource.pData, so_dataPtr, sizeof(SOBufferType));

	cout<< "CamPos||X:" <<m_Emit_Position.x << "||Y:"<<m_Emit_Position.y <<"||Z:"<<m_Emit_Position.z<<"||\n";
	// Unlock the constant buffer.
    deviceContext->Unmap(m_SO_constant_buffer, 0);


//	deviceContext->UpdateSubresource(m_SO_constant_buffer, 0, 0, &data, 0, 0);

 //   D3D11_MAPPED_SUBRESOURCE mappedResource;
	//WorldMatrixBuffer* dataPtr ;//= new WorldMatrixBuffer;
	//unsigned int bufferNumber;
	//// Lock the constant buffer so it can be written to.
	//result = deviceContext->Map(m_SO_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//if(FAILED(result))
	//{
	//	return false;
	//}

	//// Get a pointer to the data in the constant buffer.
	//dataPtr = (WorldMatrixBuffer*)mappedResource.pData;

	//// Copy the matrices into the constant buffer.
	//dataPtr->gViewProj = *ViewProj;
	//dataPtr->gEyePosition = camEyePos;
	//dataPtr->gap = gameTime;

	//
	////memcpy(so_mappedResource.pData, dataPtr, sizeof(WorldMatrixBuffer));
	//
	//// Unlock the constant buffer.
 //   deviceContext->Unmap(m_SO_constant_buffer, 0);


	//ID3D11Buffer* buffers [2] = {m_SO_constant_buffer, m_world_matrix_buffer};
	// Set the position of the constant buffer in the vertex shader.
	//unsigned int bufferNumber [2] = {0,1};
	
	////////////////////////////////////////////////////////////////
	// Vertex Shader Constant Resources Set HERE
   // deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_particle_parameter_buffer);
	//////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// Geometry Shader Constant Resources Set HERE
	deviceContext->GSSetConstantBuffers(0, 1, &m_SO_constant_buffer);
	//deviceContext->GSSetConstantBuffers(0, 1, &m_world_matrix_buffer);
	deviceContext->GSSetShaderResources(0, 1, &m_RandomTexSRV_Shader);
	deviceContext->GSSetSamplers(0, 1, &m_sampleState_Shader);
	/////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////
	// Geometry Shader Constant Resources Set HERE
//	deviceContext->PSSetShaderResources(0, 1, &m_FireTexture);
	//deviceContext->PSSetSamplers(0, 1, &m_sampleState_Shader);
	/////////////////////////////////////////////////////////////////

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////


bool RainParticleSystem::SetShaderParameters_Draw(ID3D11DeviceContext* deviceContext,
															 float timeStep, float gameTime,
															 XMFLOAT3 camEyePos, D3DXMATRIX* ViewProj )
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	WorldMatrixBuffer* dataPtr;
	unsigned int bufferNumber;
	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_world_matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (WorldMatrixBuffer*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->gViewProj = *ViewProj;
	dataPtr->gEyePosition = camEyePos;
	dataPtr->gap = gameTime;
	

	// Unlock the constant buffer.
    deviceContext->Unmap(m_world_matrix_buffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;
	
	////////////////////////////////////////////////////////////////
	// Vertex Shader Constant Resources Set HERE
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_world_matrix_buffer);
	//////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// Geometry Shader Constant Resources Set HERE
	deviceContext->GSSetConstantBuffers(bufferNumber, 1, &m_world_matrix_buffer);
	/////////////////////////////////////////////////////////////////
	ID3D11ShaderResourceView* arrayTexture[2] = {m_FireTexture,m_RandomTexSRV_Shader};
	///////////////////////////////////////////////////////////////
	// Geometry Shader Constant Resources Set HERE
	deviceContext->PSSetShaderResources(0, 2, arrayTexture);
	//deviceContext->PSSetShaderResources(1, 1, &m_FireTexture);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState_Shader);
	/////////////////////////////////////////////////////////////////

	return true;
}



void RainParticleSystem::RenderShader_StreamOut(ID3D11DeviceContext* deviceContext)
{
	//HRESULT result;

	unsigned int stride;
	unsigned int offset ;
	ID3D11Buffer* bufferPointers;
	ID3D11Buffer* bufferArrayNull[1] = {0};
	// Set the buffer strides.
	stride = sizeof(Particle);

	// Set the buffer offsets.
	offset = 0;

	////////////////////////////////////////////////////////////////////
	// Input Assembler Stage
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->IASetInputLayout(m_layout_streamOut);
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	// Vertex Shader Stage
	deviceContext->VSSetShader(m_vertexShader_StreamOut, NULL, 0);
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	// Geometry Shader Stage
	deviceContext->GSSetShader(m_geometryShader_StreamOut, NULL, 0);
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	// Stream-out Stage
	deviceContext->SOSetTargets(1, &m_StreamOutBuffer, &offset);
	////////////////////////////////////////////////////////////////////


	
	////////////////////////////////////////////////////////////////////
	// Stream-out Stage
	deviceContext->PSSetShader(NULL, NULL, 0);
	////////////////////////////////////////////////////////////////////
	

    
	if(flag_FirstRun)
	{
		bufferPointers = m_initBuffer;
		// Set the vertex buffer to active in the input assembler so it can be rendered.
		deviceContext->IASetVertexBuffers(0, 1, &bufferPointers, &stride, &offset);
		deviceContext->Draw(1,0);
		deviceContext->SOSetTargets(1, bufferArrayNull, 0);
		deviceContext->IASetVertexBuffers(0, 1, bufferArrayNull, &stride, &offset);
		flag_FirstRun = false;
	}
	else
	{
		bufferPointers = m_DrawBuffer;
		deviceContext->IASetVertexBuffers(0, 1, &bufferPointers, &stride, &offset);
		deviceContext->DrawAuto();
		deviceContext->SOSetTargets(1, bufferArrayNull, 0);
		deviceContext->IASetVertexBuffers(0, 1, bufferArrayNull, &stride, &offset);
	}

	swap(m_DrawBuffer, m_StreamOutBuffer);
		
}

bool RainParticleSystem::RenderShader_Draw(ID3D11DeviceContext* deviceContext)
{
	unsigned int strides;
	unsigned int offsets;

	strides = sizeof(Particle); 	

	// Set the buffer offsets.
	offsets = 0;
	
	//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGL);

	ID3D11Buffer* bufferArray[1] = {0};

	deviceContext->SOSetTargets(1, bufferArray, 0);
	//deviceContext->PSInput(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	deviceContext->IASetVertexBuffers(0, 1, &m_DrawBuffer, &strides, &offsets);
	
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout_Draw);

    // Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader_Draw, NULL, 0);
	deviceContext->GSSetShader(m_geometryShader_Draw, NULL, 0);
   // deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	
	deviceContext->PSSetShader(m_pixelShader_Draw, NULL, 0);
	// Set the sampler state in the pixel shader.
	//deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawAuto();//(vertexCount, instanceCount, 0, 0);

		deviceContext->IASetVertexBuffers(0, 1, bufferArray, &strides, &offsets);
	return true;
}




bool RainParticleSystem::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	//m_MaxParticles  = 10000;
	flag_FirstRun = true;
	// Initialize the vertex and pixel shaders.
	result = InitializeShader_StreamOut(device, hwnd, L"RainGeometryShader.hlsl", L"GeometryShaderFireTest.hlsl" ,L"RainGeometryShader.hlsl");
	if(!result)
	{
		MessageBox(hwnd, STR_CLASSNAME, L"InitializeShader_StreamOut:: FAILED", MB_OK);
		return false;
	}

	result = InitializeShader_Draw(device, hwnd, L"RainPixelShader.hlsl", L"RainPixelShader.hlsl" ,L"RainPixelShader.hlsl");
	if(!result)
	{
		MessageBox(hwnd, STR_CLASSNAME, L"InitializeShader_Draw:: FAILED", MB_OK);
		return false;
	}


	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. Comple Shaders VS, GS for StreamOut Stage (m_vertexShader_StreamOut|m_geometryShader_StreamOut)
// 2. Create InputLayout for VS stage as struct Particle |vertex buffer bind type|(m_layout_streamOut)
// 3. Create Vertex Buffer m_initBuffer m_DrawBuffer m_StreamOutBuffer
// 4. Create Constant/Uniform Buffer for m_particle_parameter_buffer|m_sampleState_Shader|m_RandomTexSRV_Shader.
bool RainParticleSystem::InitializeShader_StreamOut(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename,
														  WCHAR* gsFilename ,WCHAR* psFilename )
{

	HRESULT result;
	ID3D10Blob* errorMessage;

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. Comple Shaders VS, GS for StreamOut Stage (m_vertexShader_StreamOut|m_geometryShader_StreamOut)
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* geometryShaderBuffer;
	

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	

    // Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "StreamOutVS", "vs_5_0",
							D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	result = D3DX11CompileFromFile(gsFilename, NULL, NULL, "main", "gs_5_0", D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL, 
								   &geometryShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, gsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, gsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		NULL, &m_vertexShader_StreamOut);
	if(FAILED(result))
	{
		return false;
	}


	////////////////////////////////////////////////////////////////////
	///Create Geometry Shader with Stream-out
	///////////////////////////////////////////////////////////////////

	D3D11_SO_DECLARATION_ENTRY pDecl[] =
	{
		//// semantic name, semantic index, start component, component count, output slot
		{ 0 ,"POSITION" , 0, 0, 3, 0 },   // output all components of position
		{ 0 ,"VELOCITY" , 0, 0, 3, 0 }, 	     // output the first 2 texture coordinates
		{ 0 ,"SIZE"     , 0, 0, 2, 0 }, 	     // output the first 2 texture coordinates
		{ 0 ,"AGE"      , 0, 0, 1, 0 },  	     // output the first 2 texture coordinates
		{ 0 ,"TYPE"     , 0, 0, 1, 0 }, 	     // output the first 2 texture coordinates

			// semantic name, semantic index, start component, component count, output slot
		//{ 0 ,"POSITION", 0, 0, 3, 0 },   // output all components of position
		//{ 0 ,"VELOCITY"  , 0, 0, 3, 0 }, 	     // output the first 2 texture coordinates
	};


	result = device->CreateGeometryShaderWithStreamOutput( geometryShaderBuffer->GetBufferPointer(), 
		geometryShaderBuffer->GetBufferSize(), pDecl, 5, NULL, 
		0,  D3D11_SO_NO_RASTERIZED_STREAM, NULL, &m_geometryShader_StreamOut );

	if(FAILED(result))
	{
		return false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. Create InputLayout for VS stage as struct Particle |vertex buffer bind type|(m_layout_streamOut)
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	result = createInputLayoutDesc_StreamOut(device, vertexShaderBuffer);

	if(FAILED(result))
	{
		return false;
	}
	
	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	geometryShaderBuffer->Release();
	geometryShaderBuffer = 0;

	//////////////////////////////////////////////////////////////////////////////////
	// 3. Create Vertex Buffer m_initBuffer m_DrawBuffer m_StreamOutBuffer
	//////////////////////////////////////////////////////////////////////////////////
	RainParticleSystem::InitVertextBuffers(device);
	////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 4. Create Constant/Uniform Buffer for m_particle_parameter_buffer|m_sampleState_Shader|m_RandomTexSRV_Shader.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	createConstantBuffer_StreamOut(device);
	/////////////////////////////////////////////////////////////////////////////////////

	return true;
}

bool RainParticleSystem::InitVertextBuffers(ID3D11Device* device)
{
	

	///////////////////////////////////////////////////////////////////////
	// Create Vertex buffer for initial injection of the emiter particle
	// 1. m_initBuffer
	///////////////////////////////////////////////////////////////////////

	Particle* particle[5];
	D3D11_BUFFER_DESC particleBufferDesc;
    D3D11_SUBRESOURCE_DATA particleDataResource;
	HRESULT result;

	// Set the number of vertices in the vertex array.
	m_vertexCount = 5;

	// Create the vertex array.
	particle[0]  = new Particle;	
	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	//ZeroMemory(&particle, sizeof(Particle));
	particle[0] ->Size =  XMFLOAT2(0.0,10.0);
	particle[0] ->InitialVel = XMFLOAT3(3.0,0.0,0.0);
	particle[0] ->InitialPos = XMFLOAT3(12.0,32.0,0.0);
	particle[0] ->Age  = 0.0f;
	particle[0]->Type = 0; 
	
		particle[1]   = new Particle;	
	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	//ZeroMemory(&particle, sizeof(Particle));
	particle[1] ->Size =  XMFLOAT2(0.0,10.0);
	particle[1] ->InitialVel = XMFLOAT3(3.0,0.0,0.0);
	particle[1] ->InitialPos = XMFLOAT3(12.0,32.0,0.0);
	particle[1] ->Age  = 0.0f;
	particle[1] ->Type = 0; 

		particle[2]   = new Particle;	
	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	//ZeroMemory(&particle, sizeof(Particle));
	particle[2] ->Size =  XMFLOAT2(0.0,10.0);
	particle[2]->InitialVel = XMFLOAT3(3.0,0.0,0.0);
	particle[2]->InitialPos = XMFLOAT3(12.0,32.0,0.0);
	particle[2]->Age  = 0.0f;
	particle[2]->Type = 0; 

		particle[3]  = new Particle;	
	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	//ZeroMemory(&particle, sizeof(Particle));
	particle[3]->Size =  XMFLOAT2(0.0,10.0);
	particle[3]->InitialVel = XMFLOAT3(3.0,0.0,0.0);
	particle[3]->InitialPos = XMFLOAT3(12.0,32.0,0.0);
	particle[3]->Age  = 0.0f;
	particle[3]->Type = 0; 

	particle[4]  = new Particle;	
	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	//ZeroMemory(&particle, sizeof(Particle));
	particle[4]->Size =  XMFLOAT2(0.0,10.0);
	particle[4]->InitialVel = XMFLOAT3(3.0,0.0,0.0);
	particle[4]->InitialPos = XMFLOAT3(12.0,32.0,0.0);
	particle[4]->Age  = 0.0f;
	particle[4]->Type = 0; 


    particleBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    particleBufferDesc.ByteWidth = sizeof(Particle) * m_vertexCount;
    particleBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    particleBufferDesc.CPUAccessFlags = 0;
    particleBufferDesc.MiscFlags = 0;
	particleBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    particleDataResource.pSysMem = *particle;
	particleDataResource.SysMemPitch = 0;
	particleDataResource.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&particleBufferDesc, &particleDataResource, &m_initBuffer);
	if(FAILED(result))
	{
		return false;
	}
	
	// Release the vertex array now that the vertex buffer has been created and loaded.
//	delete particle;	
	//particle = 0;

	///////////////////////////////////////////////////////////////////////
	// Create Stream-out buffer instance without binding any data
	// 1. m_StreamOutBuffer.
	// 2. m_DrawBuffer.
	///////////////////////////////////////////////////////////////

	int m_nBufferSize = m_MaxParticles;

		D3D11_BUFFER_DESC bufferDesc =
		{
			m_nBufferSize,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_STREAM_OUTPUT,
			0,
			0,
			0
		};

	// Set up the description of the instance buffer.
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(Particle) * m_nBufferSize;
	bufferDesc.BindFlags =  D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer( &bufferDesc, NULL, &m_StreamOutBuffer );

		if(FAILED(result))
	{
		return false;
	}

	result = device->CreateBuffer( &bufferDesc, NULL, &m_DrawBuffer );

		if(FAILED(result))
	{
		return false;
	}

		D3D11_BUFFER_DESC soDesc;


soDesc.ByteWidth			= sizeof(Particle) *m_nBufferSize; // 10mb
soDesc.MiscFlags			= 0;
soDesc.StructureByteStride	= 0;
soDesc.BindFlags		= 0;
soDesc.CPUAccessFlags	= D3D11_CPU_ACCESS_READ;
soDesc.Usage			= D3D11_USAGE_STAGING;

result = device->CreateBuffer( &soDesc, NULL, &m_StagingStreamOutBuffer );

	
	if(FAILED(result))
	{
		return false;
	}

		return true;
}

bool RainParticleSystem::createInputLayoutDesc_StreamOut(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer )
{
	HRESULT result = true;
	const D3D11_INPUT_ELEMENT_DESC m_Particle[5] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
		
	unsigned int numElements;


	// Get a count of the elements in the layout.
    numElements = sizeof(m_Particle) / sizeof(m_Particle[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(m_Particle, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout_streamOut);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// 1. Create a particle buffer to send parameters for particle system stream-out shader
// 2. Create a random texture resource for random function in shader
// 3. Create a Sample Descriptor for pixel shader or sampling form texture
bool RainParticleSystem::createConstantBuffer_StreamOut(ID3D11Device* device)
{
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC particleBufferDesc;	
	HRESULT result = true;
	
	int size0 = sizeof(SOBufferType);
	int size1 = sizeof(XMFLOAT3);
	int size3 = sizeof(float);
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    particleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	particleBufferDesc.ByteWidth = sizeof(SOBufferType);
    particleBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    particleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    particleBufferDesc.MiscFlags = 0;
	particleBufferDesc.StructureByteStride = 0;

	
	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&particleBufferDesc, NULL, &m_SO_constant_buffer);
	if(FAILED(result))
	{
		result = false;
	}

	m_RandomTexSRV_Shader = m_ShaderUtility->CreateRandomTexture1DSRV(device);

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
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState_Shader);
	if(FAILED(result))
	{
		return false;
	}
	
	return true;
}
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. Comple Shaders VS, GS for StreamOut Stage (m_vertexShader_Draw|m_geometryShader_Draw|m_PixelShader_Draw)
// 2. Create InputLayout for VS stage as struct Particle |vertex buffer bind type|(m_layout_Draw)
bool RainParticleSystem::InitializeShader_Draw(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, 
													 WCHAR* gsFilename ,WCHAR* psFilename)
{

	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* geometryShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;


	
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. Comple Shaders VS, GS for StreamOut Stage (m_vertexShader_StreamOut|m_geometryShader_StreamOut)
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	geometryShaderBuffer = 0;
	pixelShaderBuffer = 0;

    // Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "DrawVS", "vs_5_0", D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	  // Compile the pixel shader code.
	result = D3DX11CompileFromFile(gsFilename, NULL, NULL, "DrawGS", "gs_5_0", D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL, 
								   &geometryShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "DrawPS", "ps_5_0", D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL, 
								   &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}
	
	

    // Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader_Draw);
	if(FAILED(result))
	{
		return false;
	}

	    // Create the pixel shader from the buffer.
	result = device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), NULL, &m_geometryShader_Draw);
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader_Draw);
	if(FAILED(result))
	{
		return false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. Create InputLayout for VS stage as struct Particle |vertex buffer bind type|(m_layout_Draw)
	////////////////////////////////////////////////////////////////////////////////////////////////

	result = createInputLayoutDesc_Draw(device, vertexShaderBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	geometryShaderBuffer->Release();
	geometryShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;
	
	result = createConstantBuffer_Draw(device);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

//////////////////////////////
// DRAW SHADER FUNCTIONS
//////////////////////////////

bool RainParticleSystem::createConstantBuffer_Draw(ID3D11Device* device)
{
	D3D11_BUFFER_DESC worldMatrixBufferDesc;	
	HRESULT result = true;
	int size1 = sizeof(WorldMatrixBuffer);
	int size2 = sizeof(D3DXMATRIX);
	int size3 = sizeof(XMFLOAT3);
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    worldMatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	worldMatrixBufferDesc.ByteWidth = sizeof(WorldMatrixBuffer);
    worldMatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    worldMatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    worldMatrixBufferDesc.MiscFlags = 0;
	worldMatrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&worldMatrixBufferDesc, NULL, &m_world_matrix_buffer);
	if(FAILED(result))
	{
		result = false;
	}

	m_FireTexture =  m_ShaderUtility->CreateTextureFromFile(device, L"raindrop.dds");

	return true;
}

bool RainParticleSystem::createInputLayoutDesc_Draw(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer )
{
		HRESULT result = true;
	const D3D11_INPUT_ELEMENT_DESC m_Particle[5] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
		
	unsigned int numElements;


	// Get a count of the elements in the layout.
    numElements = sizeof(m_Particle) / sizeof(m_Particle[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(m_Particle, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout_Draw);
	if(FAILED(result))
	{
		return false;
	}


	return true;
}





void RainParticleSystem::Shutdown()
{
	ShutdownShader_StreamOut();
	ShutdownShader_Draw();
}


void RainParticleSystem::ShutdownShader_StreamOut()
{


}



void RainParticleSystem::ShutdownShader_Draw()
{


}


void RainParticleSystem::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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
	for(i=0; i<bufferSize; i++)
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

///////////////////////////////////////////////////////////////////////////
// Getter and Setter Methods implementaion
///////////////////////////////////////////////////////////////////////////
void RainParticleSystem::setEmit_Position(XMFLOAT3 position)
{
	m_Emit_Position = position;
}

void RainParticleSystem::setCamera_Eye_Position(XMFLOAT3 position)
{
	m_EyePosW = position;
}

	/////////////////////////////////////////////////////////////////////////////////
	// DEBUG
//	deviceContext->CopyResource( m_StagingStreamOutBuffer, m_StreamOutBuffer );
//
//D3D11_MAPPED_SUBRESOURCE data;
//deviceContext->Map( m_StagingStreamOutBuffer, 0, D3D11_MAP_READ, 0, &data ) ;
//
//	struct GS_OUTPUT
//	{
//		D3DXVECTOR3 COLOUR;
//		D3DXVECTOR3 DOMAIN_SHADER_LOCATION;
//		D3DXVECTOR3 WORLD_POSITION;
//	};
//
//	Particle *pRaw = reinterpret_cast< Particle* >( data.pData );
//
//
//	/* Work with the pRaw[] array here */
//	// Consider StringCchPrintf() and OutputDebugString() as simple ways of printing the above struct, or use the debugger and step through.
//	
//	deviceContext->Unmap( m_StagingStreamOutBuffer, 0 );
//
	//////////////////////////////////////////////////////////////////////////////////

//	swap(m_DrawBuffer, m_StreamOutBuffer);


