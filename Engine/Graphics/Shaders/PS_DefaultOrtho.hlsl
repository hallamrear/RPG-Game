#include "Structures.hlsli"

Texture2D TextureList[MAX_LOADABLE_TEXTURES] : register(t0, space0);
SamplerState linearSampler : register(s0);

float4 main(VS_UI_IMAGE_VERTEX_OUTPUT input) : SV_TARGET
{
    return TextureList[TextureIndex].SampleLevel(linearSampler, input.UV, 0);
}