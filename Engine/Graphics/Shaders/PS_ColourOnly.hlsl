#include "Structures.hlsli"

float4 main(VS_COLOUR_ONLY_OUTPUT input) : SV_TARGET
{
    return input.Colour;
}