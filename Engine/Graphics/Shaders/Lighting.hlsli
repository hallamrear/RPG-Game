#ifndef __LIGHTING_HLSL__
#define __LIGHTING_HLSL__

#include "Structures.hlsli"

float CalculateAttenuation(float distance, float constantAtt, float linearAtt, float quadraticAtt)
{
	return 1.0f / (constantAtt + linearAtt * distance + quadraticAtt * (distance * distance));
}

//Lambert's Cosine Law
float CosineLaw(float3 toLightVector, float3 normal)
{
    return saturate(dot(normalize(toLightVector), normalize(normal)));
}

float4 CalculateAmbientLighting()
{
    return float4(0.25f, 0.25f, 0.25f, 1.0f);
}

float4 CalculateDirectionalLighting(Light light, float4 sampleColour, float3 worldPosition, float3 normal)
{
    float3 toLight = normalize(-light.Direction.xyz);
    
    float NdotL = CosineLaw(normalize(toLight), normalize(normal));
    float3 diffuse = light.Diffuse * NdotL * sampleColour;
    
    float3 toEye = normalize(CameraPositionW.xyz - worldPosition.xyz);    
    float3 halfway = normalize(toEye + toLight);
    float NdotH = CosineLaw(normalize(halfway), normalize(normal));
    float3 specular = light.Specular * pow(NdotH, 256.0f) * 32.0f * sampleColour;
    
    return float4(diffuse + specular, 1.0f);
}

float4 CalculatePointLighting(Light light, float4 sampleColour, float3 worldPosition, float3 normal)
{
    float3 toLight = light.Position.xyz - worldPosition;

    float NdotL = CosineLaw(normalize(toLight), normalize(normal));
    float3 diffuse = light.Diffuse * NdotL * sampleColour;

    float3 toEye = normalize(CameraPositionW.xyz - worldPosition.xyz);    
    float3 halfway = normalize(toEye + toLight);
    float NdotH = CosineLaw(normalize(halfway), normalize(normal));
    float3 specular = light.Specular * pow(NdotH, light.SpecularPower) * sampleColour;

    float distance = length(toLight);
    float attenuation = CalculateAttenuation(distance, light.Attenuation.x, light.Attenuation.y, light.Attenuation.z);

    diffuse *= attenuation;
    specular *= attenuation;    

    return float4(diffuse + specular, 1.0f);
}

float4 CalculateSpotLight(Light light, float4 sampleColour, float3 worldPosition, float3 normal)
{
    float3 toLight = light.Position.xyz - worldPosition;

    float NdotL = CosineLaw(normalize(toLight), normalize(normal));
    float3 diffuse = light.Diffuse * NdotL * sampleColour;

    float3 toEye = normalize(CameraPositionW.xyz - worldPosition.xyz);    
    float3 halfway = normalize(toEye + toLight);
    float NdotH = CosineLaw(normalize(halfway), normalize(normal));
    float3 specular = light.Specular * pow(NdotH, 256.0f) * 32.0f * sampleColour;

    float theta = dot(normalize(toLight), normalize(-light.Direction));
    float epsilon = (light.InnerCutoff - light.OuterCutoff);
    float intensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0f, 1.0f);
    diffuse *= intensity;
    specular *= intensity;    


    float distance = length(toLight);
    float attenuation = CalculateAttenuation(distance, light.Attenuation.x, light.Attenuation.y, light.Attenuation.z);

    diffuse *= attenuation;
    specular *= attenuation;    

    return float4(diffuse + specular, 1.0f);
}

float4 CalculateLighting(float4 sampleColour, float3 worldPosition, float3 normal)
{
    float4 lightColour = CalculateAmbientLighting();
    
	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        Light light = LightData[i];
      
        if(light.Enabled == 0)
            continue;

        switch (light.Type)
        {
            case LIGHT_DIRECTIONAL:
                lightColour += CalculateDirectionalLighting(light, sampleColour, worldPosition, normal);
                break;
          
            case LIGHT_POINT:
                lightColour += CalculatePointLighting(light, sampleColour, worldPosition, normal);
                break;
          
            case LIGHT_SPOT:
                lightColour += CalculateSpotLight(light, sampleColour, worldPosition, normal);
                break;            
          
            default:
                break;
        }
    }

	return lightColour;
}

#endif //__LIGHTING_HLSL__