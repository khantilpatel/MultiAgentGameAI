////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SIMPLESHADERCLASS_H_
#define _SIMPLESHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: TextureShaderClass
////////////////////////////////////////////////////////////////////////////////
class SimpleShaderClass
{
	struct VertexType
	{
		D3DXVECTOR4 position;
	    D3DXVECTOR2 texture;
	};

public:
	SimpleShaderClass();
	SimpleShaderClass(const SimpleShaderClass&);
	~SimpleShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, int,ID3D11Buffer*, ID3D11ShaderResourceView*);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*,WCHAR*);
	bool createInputLayoutDesc(ID3D11Device* device, ID3D10Blob* vertexShaderBuffer);
	bool createConstantBuffer_TextureBuffer(ID3D11Device* device);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*,  ID3D11ShaderResourceView*);
	void RenderShader(ID3D11DeviceContext*, int, int, ID3D11Buffer* );

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11ComputeShader* m_computeShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;


};

#endif