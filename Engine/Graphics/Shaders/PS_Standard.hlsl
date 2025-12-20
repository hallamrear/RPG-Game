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
    //float4 lightColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    //Light light;
    //Material mat = MaterialData;
    
    //float3 worldPosition = input.PositionW.xyz;
    //float3 lightingNormal = normalize(input.NormalW);
    
    //float4 sampleColour = mat.BaseColour * DiffuseTexture.SampleLevel(linearSampler, input.UV, 0);
        
    //for (int i = 0; i < MAX_LIGHT_COUNT; i++)
    //{
    //    light = LightData[i];
        
    //    if(light.Enabled == 0)
    //        continue;
        
    //    switch (light.Type)
    //    {
    //        case LIGHT_DIRECTIONAL:
    //            lightColour += CalculateDirectionalLight(light, mat, sampleColour, lightingNormal, worldPosition);
    //            break;
            
    //        case LIGHT_POINT:
    //            lightColour += CalculatePointLight(light, mat, sampleColour, lightingNormal, worldPosition);
    //            break;
            
    //        case LIGHT_SPOT:
    //            lightColour += CalculateSpotLight(light, mat, sampleColour, lightingNormal, worldPosition);
    //            break;            
            
    //        default:
    //            break;
    //    }
    //}
    
    float4 lightColour = input.Colour;
    return float4(lightColour.rgb, 1.0f);
}