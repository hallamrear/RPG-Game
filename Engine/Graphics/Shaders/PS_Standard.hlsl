#include "Structures.hlsli"
#include "Lighting.hlsli"

SamplerState linearSampler : register(s0);

float4 TriplanarSample(float3 worldPos, float3 worldNormal, float scale)
{
    // Take absolute values of the normal to determine weights
    float3 blendWeights = abs(worldNormal);
    
    // Make weights sum to 1.0
    blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);
    
    Texture2D xTex = ResourceDescriptorHeap[TextureIndex[0]];
    Texture2D yTex = ResourceDescriptorHeap[TextureIndex[1]];
    Texture2D zTex = ResourceDescriptorHeap[TextureIndex[2]];
    
    // Sample texture projections on each axis
    float4 xProj = xTex.SampleLevel(linearSampler, worldPos.yz * scale, 0);
    float4 yProj = yTex.SampleLevel(linearSampler, worldPos.xz * scale, 0);
    float4 zProj = zTex.SampleLevel(linearSampler, worldPos.xy * scale, 0);
    
    // Blend projections based on weights
    return xProj * blendWeights.x + yProj * blendWeights.y + zProj * blendWeights.z;
}

float4 Bands(float2 UV)
{
    // Scale the V coordinate by 3 to make textures tile perfectly inside their band
    // Using frac() ensures the texture repeats correctly from 0.0 to 1.0 within its slot
    float2 tiledUV = float2(UV.x, frac(UV.y * 3.0));

    Texture2D TextureBand1 = ResourceDescriptorHeap[TextureIndex[0]];
    Texture2D TextureBand2 = ResourceDescriptorHeap[TextureIndex[1]];
    Texture2D TextureBand3 = ResourceDescriptorHeap[TextureIndex[2]];
    
    // Sample all three textures at the calculated tiled UV coordinates
    float4 color1 = TextureBand1.SampleLevel(linearSampler, tiledUV, 0);
    float4 color2 = TextureBand2.SampleLevel(linearSampler, tiledUV, 0);
    float4 color3 = TextureBand3.SampleLevel(linearSampler, tiledUV, 0);

    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Conditional branching based on the vertical UV position
    if (tiledUV.y < 0.333)
    {
        finalColor = color1;
    }
    else if (tiledUV.y < 0.666)
    {
        finalColor = color2;
    }
    else
    {
        finalColor = color3;
    }

    return finalColor;
}

float4 main(VS_STANDARD_VERTEX_OUTPUT input) : SV_TARGET
{
    float4 sampleColour = MaterialData.BaseColour;
    
    return Bands(input.UV);
    
    if (TextureIndex[0] > 0)
    {
        float scale = 100.0f;
        
        // Take absolute values of the normal to determine weights
        float3 blendWeights = abs(input.NormalW.xyz);
    
        // Make weights sum to 1.0
        blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);
    
        Texture2D xTex = ResourceDescriptorHeap[TextureIndex[0]];
        Texture2D yTex = ResourceDescriptorHeap[TextureIndex[1]];
        Texture2D zTex = ResourceDescriptorHeap[TextureIndex[2]];
    
        // Sample texture projections on each axis
        float4 xProj = xTex.SampleLevel(linearSampler, input.PositionW.yz * scale, 0);
        float4 yProj = yTex.SampleLevel(linearSampler, input.PositionW.xz * scale, 0);
        float4 zProj = zTex.SampleLevel(linearSampler, input.PositionW.xy * scale, 0);
    
        // Blend projections based on weights
        //sampleColour = xProj * blendWeights.x; //+s
        sampleColour = yProj * blendWeights.y;
        //sampleColour = zProj * blendWeights.z;
        //sampleColour = TriplanarSample(input.PositionW.xyz, input.NormalW.xyz, 1.0f);
    }

    float4 lightColour = CalculateLighting(sampleColour, input.PositionW.xyz, input.NormalW);
    
    return float4(lightColour.rgb, 1.0f);
}