cbuffer TimeBuffer : register(b4)
{
    float time;       // verstrichene Zeit in Sekunden
    float3 padding;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
};

// Einfache HSV -> RGB Konvertierung
float3 HsvToRgb(float h, float s, float v)
{
    float c  = v * s;
    float x  = c * (1.0f - abs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m  = v - c;

    float3 rgb;
    if      (h <  60.0f) rgb = float3(c, x, 0);
    else if (h < 120.0f) rgb = float3(x, c, 0);
    else if (h < 180.0f) rgb = float3(0, c, x);
    else if (h < 240.0f) rgb = float3(0, x, c);
    else if (h < 300.0f) rgb = float3(x, 0, c);
    else                 rgb = float3(c, 0, x);

    return rgb + m;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 wp = input.worldPos;

    // Plasma-Wellenformel: Ueberlagerung mehrerer Sinuswellen im Raum
    float wave1 = sin(wp.x * 2.0f + time * 1.5f);
    float wave2 = sin(wp.y * 2.5f + time * 1.1f);
    float wave3 = sin(wp.z * 3.0f - time * 0.9f);
    float wave4 = sin((wp.x + wp.z) * 1.8f + time * 2.0f);

    // Plasma-Wert zwischen 0 und 1
    float plasma = (wave1 + wave2 + wave3 + wave4) * 0.25f; // -1 .. +1
    plasma = plasma * 0.5f + 0.5f;                           //  0 .. 1

    // Hue ueber den gesamten Farbkreis rotieren lassen
    float hue = fmod(plasma * 360.0f + time * 40.0f, 360.0f);

    // Saettigung und Helligkeit: leicht pulsierende Helligkeit
    float brightness = 0.75f + 0.25f * sin(time * 3.0f + plasma * 6.28f);
    float saturation = 1.0f;

    float3 color = HsvToRgb(hue, saturation, brightness);

    // Neon-Glow: leichte Gamma-Aufhellung der hellen Bereiche
    color = pow(color, 0.7f);

    return float4(color, 1.0f);
}

