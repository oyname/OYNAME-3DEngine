#pragma once

#include <vector>
#include <string>
#include "gdxutil.h"

class Texture
{
private:
	UINT32* m_pixels;
	D3D11_TEXTURE2D_DESC m_desc;
	bool m_isLocked;

public:
	Texture();
	~Texture();

	std::wstring m_sFilename;
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_textureView;
	ID3D11SamplerState* m_imageSamplerState;

	HRESULT AddTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename);
	HRESULT CreateTexture(ID3D11Device* device, int width, int height);
	HRESULT LockBuffer(ID3D11DeviceContext* deviceContext);
	void UnlockBuffer(ID3D11DeviceContext* deviceContext);
	void SetPixel(ID3D11DeviceContext* deviceContext, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char alpha);
	void GetPixel(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& alpha);

};

typedef Texture** LPLPTEXTURE;
typedef Texture* LPTEXTURE;
typedef Texture TEXTURE;

