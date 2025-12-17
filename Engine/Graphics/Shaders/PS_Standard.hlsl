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
    float4 ambientLightColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 directLightColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    Light light;
    Material mat = MaterialData;
    
    float3 worldPosition = input.PositionW.xyz;
    float3 toEyeW = normalize(CameraPositionW.xyz - worldPosition);
    float3 lightingNormal = normalize(input.NormalW);
    
    float4 sampleColour = mat.BaseColour * DiffuseTexture.SampleLevel(linearSampler, input.UV, 0);
        
    for (int i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        light = LightData[i];
        
        if(light.Enabled == 0)
            continue;
        
        switch (light.Type)
        {
            case LIGHT_DIRECTIONAL:
                directLightColour += CalculateDirectionalLight(light, mat, lightingNormal, toEyeW);
                break;
            
            case LIGHT_POINT:
                directLightColour += CalculatePointLight(light, mat, lightingNormal, toEyeW, worldPosition);
                break;
            
            case LIGHT_SPOT:
                directLightColour += CalculateSpotLight(light, mat, lightingNormal, toEyeW, worldPosition);
                break;            
            
            default:
                break;
        }
    }

    ambientLightColour = light.Ambient * sampleColour;
    directLightColour = directLightColour * sampleColour;
    
    return ambientLightColour + directLightColour;
}