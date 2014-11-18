#include "SkyBox.h"

#include <iostream>
SkyBox::SkyBox()
{
	m_vertexShader = 0;
	m_pixelShader = 0;

	m_layout = 0;

	m_vertex_buffer = 0;

	m_index_buffer = 0;

	m_CubeMapSRV = 0;
	m_world_matrix_buffer = 0;

	m_ShaderUtility = new ShaderUtility;

	m_IndexCount = 0;
}

SkyBox::~SkyBox()
{
}

bool SkyBox::Initiaize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	//m_MaxParticles  = 10000;
	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, L"SkyPixelShader.hlsl", L"SkyPixelShader.hlsl");
	if (!result)
	{
		MessageBox(hwnd, L"SkyBox", L"InitializeShader:: FAILED", MB_OK);
		return false;
	}

	return true;
}




void SkyBox::RenderShader(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
	XMFLOAT3 camEyePos)
{
	ID3D11Buffer* bufferArrayNull[1] = { 0 };
	//XMMATRIX* worldMatrix1 = &XMMatrixTranslation(camEyePos.x, camEyePos.y, camEyePos.z);
	D3DXMATRIX worldMatrix1 = *(new D3DXMATRIX);

	D3DXMatrixTranslation(&worldMatrix1, camEyePos.x, camEyePos.y, camEyePos.z);

	//	D3DXMatrixMultiply(ViewProj, &viewMatrix, &projectionMatrix);

	D3DXMatrixTranspose(&worldMatrix1, &worldMatrix1);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	//XMMATRIX WVP = XMMatrixMultiply(T, camera.ViewProj());

	//Effects::SkyFX->SetWorldViewProj(WVP);
	//Effects::SkyFX->SetCubeMap(mCubeMapSRV);
	SetShaderParameters(deviceContext, worldMatrix1, viewMatrix, projectionMatrix);

	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertex_buffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_index_buffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->VSSetShader(m_vertexShader, NULL, 0);

	deviceContext->GSSetShader(NULL, NULL, 0);
	deviceContext->SOSetTargets(1, bufferArrayNull, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	deviceContext->DrawIndexed(m_IndexCount, 0, 0);


}

bool SkyBox::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
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

	//cout<< "CamPos||X:" <<camEyePos.x << "||Y:"<<camEyePos.y <<"||Z:"<<camEyePos.z<<"||\n";
	// Unlock the constant buffer.
	deviceContext->Unmap(m_world_matrix_buffer, 0);
	///////////////////////////////////////////////////////////////
	// Geometry Shader Constant Resources Set HERE
	deviceContext->VSSetConstantBuffers(0, 1, &m_world_matrix_buffer);
	deviceContext->PSSetShaderResources(0, 1, &m_CubeMapSRV);
	deviceContext->PSSetSamplers(0, 1, &m_sampler_state);
	/////////////////////////////////////////////////////////////////
	return true;
}

bool SkyBox::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{

	HRESULT result;
	ID3D10Blob* errorMessage;

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. Comple Shaders VS, GS for StreamOut Stage (m_vertexShader_StreamOut|m_geometryShader_StreamOut)
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	// Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "VS", "vs_5_0",
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
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


	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "PS", "ps_5_0", D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL,
		&pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
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
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}


	// Create the vertex shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(),
		NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. Create InputLayout for VS stage as struct Particle |vertex buffer bind type|(m_layout_streamOut)
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	result = createInputLayoutDesc(device, vertexShaderBuffer);

	if (FAILED(result))
	{
		return false;
	}
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	//////////////////////////////////////////////////////////////////////////////////
	// 3. Create Vertex Buffer m_initBuffer m_DrawBuffer m_StreamOutBuffer
	//////////////////////////////////////////////////////////////////////////////////
	InitVertextBuffers(device, L"sunsetcube1024.dds", 5000.0f);
	////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 4. Create Constant/Uniform Buffer for m_particle_parameter_buffer|m_sampleState_Shader|m_RandomTexSRV_Shader.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	createConstantBuffer(device);
	/////////////////////////////////////////////////////////////////////////////////////

	return true;

}

bool SkyBox::createInputLayoutDesc(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer)
{
	HRESULT result = true;
	const D3D11_INPUT_ELEMENT_DESC m_Particle[1] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};


	unsigned int numElements;


	// Get a count of the elements in the layout.
	numElements = sizeof(m_Particle) / sizeof(m_Particle[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(m_Particle, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	return true;

}


bool SkyBox::InitVertextBuffers(ID3D11Device* device, WCHAR* cubemapFilename, float skySphereRadius)
{
	HRESULT result;

	///////////////////////////////////////////////////////////////////
	// Vertex Buffer
	GeometryGenerator::MeshData sphere;
	GeometryGenerator geoGen;

	geoGen.CreateSphere(skySphereRadius, 30, 30, sphere);

	std::vector<XMFLOAT3> vertices(sphere.Vertices.size());

	for (size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		vertices[i] = sphere.Vertices[i].Position;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];

	//HR(device->CreateBuffer(&vbd, &vinitData, &m_vertex_buffer));

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vbd, &vinitData, &m_vertex_buffer);
	if (FAILED(result))
	{
		return false;
	}

	///////////////////////////////////////////////////////////////////
	// Index Buffer
	m_IndexCount = sphere.Indices.size();
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	std::vector<USHORT> indices16;
	indices16.assign(sphere.Indices.begin(), sphere.Indices.end());

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices16[0];

	result = device->CreateBuffer(&ibd, &iinitData, &m_index_buffer);
	if (FAILED(result))
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////

	m_CubeMapSRV = m_ShaderUtility->CreateTextureFromFile(device, cubemapFilename);
	return true;
}


bool SkyBox::createConstantBuffer(ID3D11Device* device)
{
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC particleBufferDesc;
	HRESULT result = true;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	particleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	particleBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	particleBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	particleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	particleBufferDesc.MiscFlags = 0;
	particleBufferDesc.StructureByteStride = 0;

	particleBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&particleBufferDesc, NULL, &m_world_matrix_buffer);
	if (FAILED(result))
	{
		result = false;
	}

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
	result = device->CreateSamplerState(&samplerDesc, &m_sampler_state);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}



void SkyBox::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	//ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	//fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i < bufferSize; i++)
	{
		std::cout << compileErrors[i];
	}

	// Close the file.
	//fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

void SkyBox::Shutdown()
{
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	if (m_pixelShader){
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	if (m_layout){
		m_layout->Release();
		m_layout = 0;
	}

	if (m_vertex_buffer){
		m_vertex_buffer->Release();
		m_vertex_buffer = 0;
	}

	if (m_index_buffer){
		m_index_buffer->Release();
		m_index_buffer = 0;
	}

	if (m_CubeMapSRV){
		m_CubeMapSRV->Release();
		m_CubeMapSRV = 0;
	}
	if (m_world_matrix_buffer){
		m_world_matrix_buffer->Release();
		m_world_matrix_buffer = 0;
	}

}