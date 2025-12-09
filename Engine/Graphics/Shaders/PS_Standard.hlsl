#include "Structures.hlsli"

SamplerState linearSampler : register(s0);

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
    return float4((input.NormalW), 1.0f);
}