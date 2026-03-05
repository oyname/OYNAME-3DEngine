#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#define RGBA(r, g, b, a) ((r << 24) | (g << 16) | (b << 8) | a)

Texture::Texture() : m_texture(nullptr), m_textureView(nullptr), m_isLocked(false)
{
    m_sFilename = L"";
}

Texture::~Texture()
{
    Memory::SafeRelease(m_imageSamplerState);
    Memory::SafeRelease(m_textureView);
    Memory::SafeRelease(m_texture);
    m_pixels = nullptr;
    m_isLocked = false;
}

HRESULT Texture::AddTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename)
{
    Memory::SafeRelease(m_imageSamplerState);
    Memory::SafeRelease(m_textureView);
    Memory::SafeRelease(m_texture);

    // Bilddaten laden
    int imageWidth, imageHeight, imageChannels;
    int desiredChannels = 4;

    char* narrowFilename = nullptr;
    size_t length = wcslen(filename) + 1; // +1 fuer das Nullzeichen am Ende
    narrowFilename = new char[length];
    size_t convertedChars = 0;
    wcstombs_s(&convertedChars, narrowFilename, length, filename, length);
    unsigned char* imageData = stbi_load(narrowFilename, &imageWidth, &imageHeight, &imageChannels, desiredChannels);
    delete[] narrowFilename;

    if (!imageData)
    {
        Debug::Log("Texture.cpp: FAIL LOAD IMAGE ", __FILE__, __LINE__);
        return E_FAIL;
    }

    // Berechnung der maximalen Groee
    UINT maxSize = imageWidth > imageHeight ? imageWidth : imageHeight;

    // Texturbeschreibung erstellen
    m_desc.Width = imageWidth;
    m_desc.Height = imageHeight;
    m_desc.MipLevels = 1;
    m_desc.ArraySize = 1;
    m_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    m_desc.SampleDesc.Count = 1;
    m_desc.SampleDesc.Quality = 0;
    m_desc.Usage = D3D11_USAGE_DEFAULT;
    m_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    m_desc.CPUAccessFlags = 0;
    m_desc.MiscFlags = 0;

    // Subressource-Daten einrichten
    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = imageData;
    subresourceData.SysMemPitch = imageWidth * 4;

    // Textur erstellen
    HRESULT hr = device->CreateTexture2D(&m_desc, &subresourceData, &m_texture);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        stbi_image_free(imageData); 
        return hr; 
    }

    // Shader Resource View erstellen
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        Memory::SafeRelease(m_texture); 
        stbi_image_free(imageData); 
        return hr; 
    }

    D3D11_SAMPLER_DESC ImageSamplerDesc = {};

    ImageSamplerDesc.Filter =  D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    ImageSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    ImageSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    ImageSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    ImageSamplerDesc.MipLODBias = 0.0f;
    ImageSamplerDesc.MaxAnisotropy = 1;
    ImageSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    ImageSamplerDesc.BorderColor[0] = 1.0f;
    ImageSamplerDesc.BorderColor[1] = 1.0f;
    ImageSamplerDesc.BorderColor[2] = 1.0f;
    ImageSamplerDesc.BorderColor[3] = 1.0f;
    ImageSamplerDesc.MinLOD = -FLT_MAX;
    ImageSamplerDesc.MaxLOD = FLT_MAX;

    hr = device->CreateSamplerState(&ImageSamplerDesc, &m_imageSamplerState);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        Memory::SafeRelease(m_texture);
        stbi_image_free(imageData); 
        return hr; 
    }

    stbi_image_free(imageData); 

    // Dateinamen speichern
    m_sFilename = filename;

    return S_OK; // Erfolg

}

HRESULT Texture::CreateTexture(ID3D11Device* device, int width, int height) 
{
    Memory::SafeRelease(m_texture);

    // Keine Initialdaten f�r eine leere Textur
    D3D11_SUBRESOURCE_DATA initData = {};

    initData.SysMemPitch = width * 4;
    initData.SysMemSlicePitch = width * height * 4;

    // Texturbeschreibung erstellen
    m_desc.Width = width;
    m_desc.Height = height;
    m_desc.MipLevels = 1;
    m_desc.ArraySize = 1;
    m_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
    m_desc.SampleDesc.Count = 1;
    m_desc.SampleDesc.Quality = 0;
    m_desc.Usage = D3D11_USAGE_DYNAMIC; 
    m_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    m_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 

    // Textur erstellen
    HRESULT hr = device->CreateTexture2D(&m_desc, nullptr, &m_texture);
    if (FAILED(hr))
    {
        // Fehlerbehandlung
        return hr;
    }

    // Shader Resource View erstellen
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        Memory::SafeRelease(m_texture);
        return hr;
    }

    D3D11_SAMPLER_DESC ImageSamplerDesc = {};

    ImageSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    ImageSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    ImageSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    ImageSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    ImageSamplerDesc.MipLODBias = 0.0f;
    ImageSamplerDesc.MaxAnisotropy = 1;
    ImageSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    ImageSamplerDesc.BorderColor[0] = 1.0f;
    ImageSamplerDesc.BorderColor[1] = 1.0f;
    ImageSamplerDesc.BorderColor[2] = 1.0f;
    ImageSamplerDesc.BorderColor[3] = 1.0f;
    ImageSamplerDesc.MinLOD = -FLT_MAX;
    ImageSamplerDesc.MaxLOD = FLT_MAX;

    hr = device->CreateSamplerState(&ImageSamplerDesc, &m_imageSamplerState);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        Memory::SafeRelease(m_texture);
        return hr;
    }

    m_texture->GetDesc(&m_desc);

    return S_OK;
}

void Texture::SetPixel(ID3D11DeviceContext* deviceContext, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char alpha)
{
    // Berechnung der Pixelposition
    UINT32 pixelValue = (alpha << 24) | (r << 16) | (g << 8) | b; // RGBA-Format
    
    if(m_isLocked)
        m_pixels[y * m_desc.Width + x] = pixelValue;

}

void Texture::GetPixel(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& alpha)
{
    if (!m_isLocked) {
        r = g = b = 255; // Wei�
        alpha = 255;
        return;
    }

    UINT32 pixelValue = m_pixels[y * m_desc.Width + x];

    alpha = (pixelValue >> 24) & 0xFF;
    r = (pixelValue >> 16) & 0xFF;
    g = (pixelValue >> 8) & 0xFF;
    b = pixelValue & 0xFF;
}

HRESULT Texture::LockBuffer(ID3D11DeviceContext* deviceContext)
{
    HRESULT hr = S_OK;

    if (m_desc.CPUAccessFlags == 0)
        return S_FALSE;

    // Daten der Textur sperren
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = deviceContext->Map(m_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    // Zugriff auf die Texturdaten
    m_pixels = (UINT32*)mappedResource.pData;

    m_isLocked = true;

    return hr;
}

void Texture::UnlockBuffer(ID3D11DeviceContext* deviceContext)
{
    deviceContext->Unmap(m_texture, 0);
    m_isLocked = false;
}