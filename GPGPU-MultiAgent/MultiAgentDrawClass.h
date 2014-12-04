////////////////////////////////////////////////////////////////////////////////
// Filename: MultiAgentDrawClass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MultiAgentDrawCLASS_H_
#define _MultiAgentDrawCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
#include "GeometryGenerator.h"
#include "ShaderUtility.h"
#include "ComputeShaderHelperClass.h"
#include "LoggerCpp/LoggerCpp.h"

using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: TextureShaderClass
////////////////////////////////////////////////////////////////////////////////
class MultiAgentDrawClass
{

	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
		XMMATRIX gridScaling;
		float frameTime;
	};

	struct Basic32
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};

	struct VertexType
	{
		D3DXVECTOR4 position;
		D3DXVECTOR2 texture;
	};

	struct ComputeShaderOut
	{
		XMFLOAT3 position;
		int type; // Type: 0 Agent; 1 Collision box
	};

	
public:
	static const int NUM_AGENTS = 4;
	static const int NUM_AGENTS_PER_BLOCK = 10;
	static const int MAP_DIMENSIONS = 8;
	int frameCount;
	

	MultiAgentDrawClass();
	MultiAgentDrawClass(const MultiAgentDrawClass&);
	~MultiAgentDrawClass();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext* device_context, HWND);
	void Shutdown();

	bool Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
		D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
		float frameTime, float gameTime, int framecount, XMFLOAT3 camEyePos,
		ID3D11UnorderedAccessView*, ID3D11UnorderedAccessView*);


	void RenderShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
		D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
		XMFLOAT3 camEyePos);

	void RenderMultipleAgentShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
		D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
		XMFLOAT3 camEyePos);

	void RenderComputeShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
		D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
		XMFLOAT3 camEyePos, ID3D11UnorderedAccessView*, ID3D11UnorderedAccessView*);
	
	void RenderComputeSpatialHashShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
		D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
		XMFLOAT3 camEyePos, ID3D11UnorderedAccessView*, ID3D11UnorderedAccessView*);


private:

	//*********TASKS*****************//
	// Common

	bool InitializeShader(ID3D11Device*, HWND);
	bool createInputLayoutDesc(ID3D11Device* device);
	bool createConstantBuffer_TextureBuffer(ID3D11Device* device);
	bool InitVertextBuffers(ID3D11Device* device, ID3D11DeviceContext* device_context);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix,
		D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, float frameTime);
	// 1. Build Spatial hash table for the map;
	ID3D11ComputeShader* m_computeShader_spatial_hash;
	// 2. CS:	interpolate the position of agents
	ID3D11ComputeShader* m_computeShader;
	// 3. VS:GS:PS: Just render the agents with cubes using geometry shader
	// 4. VS:PS:	Render floor for agents
	bool MultiAgentDrawClass::InitFloorGeometryVertextBuffers(ID3D11Device* device, ID3D11DeviceContext* device_context);


	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;

	ID3D11VertexShader* m_render_vertexShader;
	ID3D11PixelShader* m_render_pixelShader;
	ID3D11GeometryShader* m_render_geometryShader;
	ID3D11InputLayout* m_render_layout;
	


	// Texture Resource
	ID3D11SamplerState* m_sampleState;
	ID3D11ShaderResourceView* m_FloorTextureSRV;
	ID3D11ShaderResourceView* m_CubeTextureSRV;
	ID3D11ShaderResourceView* m_FloorCenterDataSRV;
	ID3D11ShaderResourceView* m_CollisionWallSRV;
	

	// Buffer data
	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;
	ID3D11Buffer* m_AgentPositionBuffer;
	ID3D11Buffer* m_AgentPositionDrawBuffer;
	ID3D11Buffer* m_Buffer_AgentCurrentPosition;
	ID3D11Buffer* m_Buffer_PsudoRandom;

	//Spatial hash
	ID3D11Buffer* m_buffer_spatial_index_table_reset;
	ID3D11Buffer* m_buffer_spatial_index_table;
	ID3D11Buffer* m_buffer_spatial_agent_id_table_reset;
	ID3D11Buffer* m_buffer_spatial_agent_id_table;

	// Constant Buffer
	ID3D11Buffer* m_world_matrix_buffer;
	ID3D11Buffer* m_Buffer_GridCenterData;



	ID3D11UnorderedAccessView*  m_view_AgentPosition_URV;
	ID3D11UnorderedAccessView* m_view_AgentCurrentPosition_URV;

	ID3D11UnorderedAccessView*  m_view_spatial_index_table_URV;
	ID3D11UnorderedAccessView* m_view_spatial_agent_id_URV;

	ID3D11UnorderedAccessView* m_view_psudo_random_URV;
	//Transformation from local to world Coordinates
	XMFLOAT4X4 mGridWorld;

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

	// Custom Classes and Utility
	ShaderUtility* m_ShaderUtility;
	ComputeShaderHelperClass* m_computeshader_helper;
};

#endif