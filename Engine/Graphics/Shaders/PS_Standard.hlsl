#include "Structures.hlsli"


Texture2D<float4> DiffuseTexture : register(t0);
Texture2D<float4> spareTex_1 : register(t1);
Texture2D<float4> spareTex_2 : register(t2);
Texture2D<float4> spareTex_3 : register(t3);
Texture2D<float4> spareTex_4 : register(t4);

SamplerState linearSampler : register(s0);

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{   
    [branch]
    if (input.UV.x < 0.5f)
        return DiffuseTexture.SampleLevel(linearSampler, input.UV, 0);
    else
        return spareTex_1.SampleLevel(linearSampler, input.UV, 0);
}