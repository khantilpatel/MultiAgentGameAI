////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{

	

public:
	struct VertexType
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

	struct InstanceType
	{
		D3DXVECTOR3 position;
	};
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();


	int GetVertexCount();
	int GetInstanceCount();
	ID3D11ShaderResourceView* GetTexture();
	ID3D11Buffer* getVertexBuffer();
	ID3D11Buffer* getInstanceBuffer();
	ID3D11Buffer* getStreamOutBuffer();

	
private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();


	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();

private:
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_instanceBuffer;
	ID3D11Buffer* m_StreamOutBuffer;

	int m_vertexCount;
	int m_instanceCount;

	TextureClass* m_Texture;
};

#endif