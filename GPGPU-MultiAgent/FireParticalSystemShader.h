////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _FIREPARTICLESYSTEMSHADERCLASS_H_
#define _FIREPARTICLESYSTEMSHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
#include <D3DCompiler.h>
#include "ShaderUtility.h"

using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: TextureShaderClass
////////////////////////////////////////////////////////////////////////////////
class FireParticalSystemShader
{
private:
	//struct MatrixBufferType
	//{
	//	D3DXMATRIX world;
	//	D3DXMATRIX view;
	//	D3DXMATRIX projection;
	//};

/*		float gTimeStep;
	float gGameTime;	
	float3 gEmitPosW;
	float3 gEyePosW;
	float4x4 gViewProj;*/ 
	struct WorldMatrixBuffer
	{
		D3DXMATRIX gViewProj;
		XMFLOAT3 gEyePosition;
		float gap;
	};

	struct SOBufferType
	{
		XMFLOAT3 gEmitPosition;
		float gTimeStep;
		XMFLOAT3 gEyePosW;
		float gGameTime;
		
	
	};

	struct Particle
	{
		XMFLOAT3 InitialPos;
		XMFLOAT3 InitialVel;
		XMFLOAT2 Size;
		float Age;
		unsigned int Type;
	};

public:
	FireParticalSystemShader();
	FireParticalSystemShader(const FireParticalSystemShader&);
	~FireParticalSystemShader();

	bool Initialize(ID3D11Device*, HWND);
	bool Render(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX ,
		float, float,XMFLOAT3);

	bool RenderShader_Draw(ID3D11DeviceContext*);
	void Shutdown();

	//Getter and Setter Methods for properties
	void setEmit_Position(XMFLOAT3 position);
	void setCamera_Eye_Position(XMFLOAT3 position);

	ID3D11Buffer* m_initBuffer;
	ID3D11Buffer* m_DrawBuffer;
	ID3D11Buffer* m_StreamOutBuffer;
	ID3D11Buffer* m_StagingStreamOutBuffer;

private:
	bool InitializeShader_StreamOut(ID3D11Device*, HWND, WCHAR*, WCHAR*,WCHAR*);
	bool InitializeShader_Draw(ID3D11Device*, HWND, WCHAR*, WCHAR*,WCHAR*);

	bool InitVertextBuffers(ID3D11Device* device);

	bool createConstantBuffer_StreamOut(ID3D11Device*);
	bool createConstantBuffer_Draw(ID3D11Device*);
	
	bool createInputLayoutDesc_StreamOut(ID3D11Device* , ID3D10Blob*);
	bool createInputLayoutDesc_Draw(ID3D11Device* , ID3D10Blob*);

	bool SetShaderParameters_StreamOut(ID3D11DeviceContext*, float , float);
	bool SetShaderParameters_Draw(ID3D11DeviceContext*, float , float, XMFLOAT3, D3DXMATRIX* );

	void RenderShader_StreamOut(ID3D11DeviceContext*);


	void ShutdownShader_StreamOut();
	void ShutdownShader_Draw();

	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);


private:
	
	ShaderUtility* m_ShaderUtility;

	// Buffer Variables



	// Shader Variables 
	ID3D11VertexShader* m_vertexShader_StreamOut;
	ID3D11PixelShader* m_pixelShader_StreamOut;
	ID3D11GeometryShader* m_geometryShader_StreamOut;

	ID3D11VertexShader* m_vertexShader_Draw;
	ID3D11PixelShader* m_pixelShader_Draw;
	ID3D11GeometryShader* m_geometryShader_Draw;

	ID3D11InputLayout* m_layout_streamOut;
	ID3D11InputLayout* m_layout_Draw;


	ID3D11Buffer* m_world_matrix_buffer;
	ID3D11Buffer* m_SO_constant_buffer;
	ID3D11SamplerState* m_sampleState_Shader;


	/////////////////////////////////////////////////////////////////////////////
	// Buffer for random function in geometry shader to generate random effects
	// on particles, for example random location of partical, velocity, size etc
	ID3D11ShaderResourceView* m_RandomTexSRV_Shader; 
	/////////////////////////////////////////////////////////////////////////////

	ID3D11ShaderResourceView* m_FireTexture;

	XMFLOAT3 m_Emit_Position;
	XMFLOAT3 m_EyePosW;
	XMFLOAT3 m_EmitDirW;

	int m_vertexCount;
	static const int m_MaxParticles = 1000;
	bool flag_FirstRun;


	
};

#endif