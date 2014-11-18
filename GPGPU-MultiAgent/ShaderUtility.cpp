#include "ShaderUtility.h"

using namespace std;


ID3D11ShaderResourceView* ShaderUtility::CreateRandomTexture1DSRV(ID3D11Device* device)
{
	HRESULT result;
	// 
	// Create the random data.
	//
	XMFLOAT4 randomValues[1024];

	for(int i = 0; i < 1024; ++i)
	{
		randomValues[i].x = MathHelper::RandF(-50.0f, 50.0f);
		randomValues[i].y = MathHelper::RandF(-50.0f, 50.0f);
		randomValues[i].z = MathHelper::RandF(-50.0f, 50.0f);
		randomValues[i].w = MathHelper::RandF(-1.0f, 50.0f);
	}

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024*sizeof(XMFLOAT4);
    initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = 1024;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

	ID3D11Texture1D* randomTex = 0;
    device->CreateTexture1D(&texDesc, &initData, &randomTex);

	//
	// Create the resource view.
	//
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;
	
	ID3D11ShaderResourceView* randomTexSRV = 0;
	 device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV);


	randomTex->Release();

	return randomTexSRV;
}



ID3D11ShaderResourceView* ShaderUtility::CreateTextureFromFile(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;

	ID3D11ShaderResourceView* texture = 0;

	// Load the texture in.
	result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &texture, NULL);
	if(FAILED(result))
	{
		return false;
	}

	return texture;
}
