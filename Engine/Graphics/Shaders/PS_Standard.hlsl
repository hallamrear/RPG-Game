#include "Structures.hlsli"
#include "Lighting.hlsli"

Texture2D TextureList[] : register(t0, space0);

SamplerState linearSampler : register(s0);

float4 TriplanarSample(float3 worldPos, float3 worldNormal, float scale)
{
    // Take absolute values of the normal to determine weights
    float3 blendWeights = abs(worldNormal);
    
    // Make weights sum to 1.0
    blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);
    
    // Sample texture projections on each axis
    float4 xProj = TextureList[TextureIndex[0]].SampleLevel(linearSampler, worldPos.yz * scale, 0);
    float4 yProj = TextureList[TextureIndex[1]].SampleLevel(linearSampler, worldPos.xz * scale, 0);
    float4 zProj = TextureList[TextureIndex[2]].SampleLevel(linearSampler, worldPos.xy * scale, 0);
    
    // Blend projections based on weights
    return xProj * blendWeights.x + yProj * blendWeights.y + zProj * blendWeights.z;
}

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
    float4 sampleColour = MaterialData.BaseColour;

    
    if (TextureIndex[0] > 0)
    {
        sampleColour = TextureList[TextureIndex[0]].SampleLevel(linearSampler, input.UV, 0);
        sampleColour = TriplanarSample(input.PositionW.xyz, input.NormalW, 1.0f);
    }

    
    float4 lightColour = CalculateLighting(sampleColour, input.PositionW.xyz, input.NormalW);
    
    return float4(lightColour.rgb, 1.0f);
}