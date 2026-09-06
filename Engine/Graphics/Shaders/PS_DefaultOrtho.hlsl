#include "Structures.hlsli"

Texture2D TextureList[MAX_LOADABLE_TEXTURES] : register(t0, space0);
SamplerState linearSampler : register(s0);

float4 main(VS_UI_IMAGE_VERTEX_OUTPUT input) : SV_TARGET
{
    if (TextureIndex[0] > 0)
    {
        return TextureList[TextureIndex[0]].SampleLevel(linearSampler, input.UV, 0);
    }
    
    return MaterialData.BaseColour;
}