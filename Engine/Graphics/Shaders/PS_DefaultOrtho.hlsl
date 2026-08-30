#include "Structures.hlsli"

Texture2D<float4> DiffuseTexture : register(t0);
SamplerState linearSampler : register(s0);

float4 main(VS_COLOUR_ONLY_OUTPUT input) : SV_TARGET
{
    float hw = 640.0f / 2.0f;
    float hh = 480.0f / 2.0f;

    float2 uv = float2(input.Position.x / hw, input.Position.y / hh);
    return DiffuseTexture.Sample(linearSampler, uv);
}