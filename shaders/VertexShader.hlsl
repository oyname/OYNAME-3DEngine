// VertexShader.hlsl - giDX Engine
// Registers: b0 (Matrices), b1 (Lights), b2 (Material), b3 (Shadow Matrices)
// VertexShader.hlsl - giDX Engine
// Registers: b0 (Matrices), b1 (Lights), b2 (Material), b3 (Shadow Matrices)

// ==================== CONSTANT BUFFERS ====================

cbuffer ConstantBuffer : register(b0)
{
    row_major float4x4 _viewMatrix;
    row_major float4x4 _projectionMatrix;
    row_major float4x4 _worldMatrix;
};

// Struktur fuer ein einzelnes Licht (muss mit C++ LightBufferData kompatibel sein!)
struct LightData
{
    float4 lightPosition; // XYZ: Position, W: 0 fuer direktional, 1 fuer positional
    float4 lightDirection; // XYZ: Direction
    float4 lightDiffuseColor; // RGB: Diffuse Farbe
    float4 lightAmbientColor; // RGB: Ambient Farbe (nur fuer erstes Licht relevant)
};

// Light-Array Buffer (SYNCHRON mit Pixel-Shader)
cbuffer LightBuffer : register(b1)
{
    LightData lights[32]; // Array von bis zu 32 Lichtern
    uint lightCount; // Aktuelle Anzahl der Lichter
    float3 lightPadding; // Padding fuer 16-Byte Alignment
};

// MaterialBuffer: 128 Bytes – identisch mit PixelShader.hlsl (b2)
cbuffer MaterialBuffer : register(b2)
{
    float4 gBaseColor;
    float4 gSpecularColor;
    float4 gEmissiveColor;
    float4 gUvTilingOffset; // xy=tiling, zw=offset

    float4 gPbr; // x=metallic y=roughness z=normalScale w=occlusionStrength
    float4 gAlpha; // x=shininess y=transparency z=alphaCutoff w=receiveShadows

    uint4 gTexIndex; // x=albedo y=normal z=orm w=decal
    uint4 gMisc; // x=blendMode y=materialFlags z,w unused
};

// ==================== SHADOW MAPPING BUFFER ====================

cbuffer ShadowMatrixBuffer : register(b3)
{
    row_major float4x4 lightViewMatrix;
    row_major float4x4 lightProjectionMatrix;
};

// ==================== INPUT / OUTPUT STRUCTURES ====================

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT; // xyz + handedness
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0; // Albedo UV
    float2 texCoord2 : TEXCOORD1; // Lightmap / Detail UV
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 tangent : TEXCOORD5; // xyz + handedness
    float3 worldPosition : TEXCOORD1;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0; // Albedo UV
    float4 positionLightSpace : TEXCOORD2;
    float3 viewDirection : TEXCOORD3;
    float2 texCoord2 : TEXCOORD4; // Lightmap / Detail UV
};

// ==================== MAIN VERTEX SHADER ====================

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    // Welt-Position berechnen
    float4 tempPosition = float4(input.position, 1.0f);
    float4 worldPos = mul(tempPosition, _worldMatrix);
    o.worldPosition = worldPos.xyz;

    // Clip-Space Position
    o.position = mul(worldPos, _viewMatrix);
    o.position = mul(o.position, _projectionMatrix);

    // Normale in den Welt-Raum transformieren (ohne Translation)
    o.normal = normalize(mul(input.normal, (float3x3) _worldMatrix));

    // Tangente in Welt-Raum (xyz) + Handedness (w)
    float3 tW = normalize(mul(input.tangent.xyz, (float3x3) _worldMatrix));
    o.tangent = float4(tW, input.tangent.w);

    // Vertex-Attribute kopieren
    o.color = input.color;
    o.texCoord = input.texCoord;
    o.texCoord2 = input.texCoord2;

    // Shadow Mapping: Welt-Position in Light-Space transformieren
    float4 lightViewPos = mul(worldPos, lightViewMatrix);
    o.positionLightSpace = mul(lightViewPos, lightProjectionMatrix);

    // Kamera-Position aus der View-Matrix extrahieren (row_major LookToLH)
    // Die View-Matrix speichert: Rows 0-2 = Rotation, Row 3 = -R*eye
    // Kamera-Position = -transpose(R) * translation
    float3 vt = float3(_viewMatrix[3][0], _viewMatrix[3][1], _viewMatrix[3][2]);
    float3 cameraPosition = float3(
        -dot(float3(_viewMatrix[0][0], _viewMatrix[0][1], _viewMatrix[0][2]), vt),
        -dot(float3(_viewMatrix[1][0], _viewMatrix[1][1], _viewMatrix[1][2]), vt),
        -dot(float3(_viewMatrix[2][0], _viewMatrix[2][1], _viewMatrix[2][2]), vt)
    );

    o.viewDirection = normalize(cameraPosition - worldPos.xyz);

    return o;
}

