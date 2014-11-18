//***************************************************************************************
// Sky.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Simple class that renders a sky using a cube map.
//***************************************************************************************

#ifndef _SKYSHADERCLASS_H_
#define _SKYSHADERCLASS_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
#include <iostream>
#include <D3DCompiler.h>
#include <vector>
#include "ShaderUtility.h"
#include "GeometryGenerator.h"



class SkyBox
{
public:
	SkyBox();
	~SkyBox();
	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};
	
	ID3D11ShaderResourceView* getCubeMapSRV();
	bool Initiaize(ID3D11Device*, HWND);


	bool InitializeShader(ID3D11Device*, HWND, WCHAR*,WCHAR*);
	void RenderShader(ID3D11DeviceContext*,D3DXMATRIX , 
									  D3DXMATRIX , D3DXMATRIX ,
									  XMFLOAT3 );
	void Shutdown();

private:
	
	bool createInputLayoutDesc(ID3D11Device* , ID3D10Blob* );
	bool createConstantBuffer(ID3D11Device* );
	//void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
	bool SetShaderParameters(ID3D11DeviceContext* , D3DXMATRIX , 
									  D3DXMATRIX , D3DXMATRIX );
	
	bool InitVertextBuffers(ID3D11Device*,  WCHAR* , float );

	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_layout;

	ID3D11Buffer* m_vertex_buffer;
	ID3D11Buffer* m_index_buffer;
	ID3D11Buffer* m_world_matrix_buffer;
	
	ID3D11SamplerState* m_sampler_state;
	ID3D11ShaderResourceView* m_CubeMapSRV;

	ShaderUtility* m_ShaderUtility;
	UINT m_IndexCount;
};

#endif // SKY_H