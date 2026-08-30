#include "Structures.hlsli"
#include "Lighting.hlsli"

Texture2D<float4> DiffuseTexture : register(t0);
Texture2D<float4> spareTex_1 : register(t1);
Texture2D<float4> spareTex_2 : register(t2);
Texture2D<float4> spareTex_3 : register(t3);
Texture2D<float4> spareTex_4 : register(t4);

SamplerState linearSampler : register(s0);

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
    //float4 sampleColour = MaterialData.BaseColour;
    //
    //if (TextureSlotEnabled[0] > 0)
    //{
    //    sampleColour *= DiffuseTexture.SampleLevel(linearSampler, input.UV, 0);
    //}

    float4 sampleColour = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 lightColour = CalculateLighting(sampleColour, input.PositionW.xyz, input.NormalW);
    return float4(lightColour.rgb, 1.0f);
}