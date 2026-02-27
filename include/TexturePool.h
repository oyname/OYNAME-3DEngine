#pragma once

#include <vector>
#include <string>
#include "gdxutil.h"

#include "Texture.h"


class TextureManager
{
private:
	typedef std::vector<LPTEXTURE> TextureContainer;
	TextureContainer tc;

	int  CheckTexture(std::wstring sFilename);

	void ReleaseTexture(void);

public:
	TextureManager();

	~TextureManager();

	static TextureManager& Instance(void);

	HRESULT LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename, LPLPTEXTURE lpTexture);
};
typedef TextureManager* LPTEXTUREMANAGER;

