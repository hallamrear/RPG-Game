#include "Structures.hlsli"

SamplerState linearSampler : register(s0);

float4 main(VS_COLOUR_ONLY_OUTPUT input) : SV_TARGET
{
    return input.Colour;
}