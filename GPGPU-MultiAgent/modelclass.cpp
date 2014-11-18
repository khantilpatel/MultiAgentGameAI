////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"


ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_instanceBuffer = 0;
	m_StreamOutBuffer= 0;
	m_Texture = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, WCHAR* textureFilename)
{
	bool result;


	// Initialize the vertex and instance buffers.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	// Load the texture for this model.
	result = LoadTexture(device, textureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	InstanceType* instances;
	D3D11_BUFFER_DESC vertexBufferDesc, instanceBufferDesc, streamOutBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, instanceData;
	HRESULT result;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Create vertex buffer instance with binding the structure of each vertex to be drawn
	/////////////////////////////////////////////////////////////////////////////////////////////

	// Set the number of vertices in the vertex array.
	m_vertexCount = 1;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Load the vertex array with data.
	vertices[0].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);  // Bottom left.
	vertices[0].texture = D3DXVECTOR2(0.0f, 1.0f);

//	vertices[1].position = D3DXVECTOR3(0.0f, 1.0f, 0.0f);  // Top middle.
//	vertices[1].texture = D3DXVECTOR2(0.5f, 0.0f);

	//vertices[2].position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);  // Bottom right.
	//vertices[2].texture = D3DXVECTOR2(1.0f, 1.0f);

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex array now that the vertex buffer has been created and loaded.
	delete [] vertices;
	vertices = 0;
	
	/////////////////////////////////////////////////////////////////////////////////////////////
	// Create instanced buffer instance with binding the position data for each new instance.
	/////////////////////////////////////////////////////////////////////////////////////////////

	// Set the number of instances in the array.
	m_instanceCount = 4;

	// Create the instance array.
	instances = new InstanceType[m_instanceCount];
	if(!instances)
	{
		return false;
	}

	// Load the instance array with data.
	instances[0].position = D3DXVECTOR3(-1.5f, -1.5f, -2.0f);
	instances[1].position = D3DXVECTOR3(-1.5f,  1.5f, 5.0f);
	instances[2].position = D3DXVECTOR3( 1.5f, -1.5f, 10.0f);
	instances[3].position = D3DXVECTOR3( 1.5f,  1.5f, 15.0f);

	// Set up the description of the instance buffer.
    instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    instanceBufferDesc.ByteWidth = sizeof(InstanceType) * m_instanceCount;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = 0;
    instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the instance data.
    instanceData.pSysMem = instances;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	// Create the instance buffer.
	result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the instance array now that the instance buffer has been created and loaded.
	delete [] instances;
	instances = 0;

	///////////////////////////////////////////////////////////////
	// Create Stream-out buffer instance without binding any data
	///////////////////////////////////////////////////////////////

	int m_nBufferSize = 1000000;

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
    bufferDesc.ByteWidth = m_nBufferSize;
	bufferDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer( &bufferDesc, NULL, &m_StreamOutBuffer );

		if(FAILED(result))
	{
		return false;
	}
	
	return true;
}


bool ModelClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
	bool result;


	// Create the texture object.
	m_Texture = new TextureClass;
	if(!m_Texture)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_Texture->Initialize(device, filename);
	if(!result)
	{
		return false;
	}

	return true;
}



int ModelClass::GetVertexCount()
{
	return m_vertexCount;
}


int ModelClass::GetInstanceCount()
{
	return m_instanceCount;
}

ID3D11Buffer* ModelClass::getVertexBuffer()
{
	return m_vertexBuffer;
}

ID3D11Buffer* ModelClass::getInstanceBuffer()
{
	return m_instanceBuffer;
}

ID3D11Buffer* ModelClass::getStreamOutBuffer()
{
	return m_StreamOutBuffer;
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}

void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if(m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}



void ModelClass::ShutdownBuffers()
{
	// Release the instance buffer.
	if(m_instanceBuffer)
	{
		m_instanceBuffer->Release();
		m_instanceBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

void ModelClass::Shutdown()
{
	// Release the model texture.
	ReleaseTexture();

	// Shutdown the vertex and instance buffers.
	ShutdownBuffers();

	return;
}

