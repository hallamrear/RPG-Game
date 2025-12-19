#ifndef __LIGHTING_HLSL__
#define __LIGHTING_HLSL__

#include "Structures.hlsli"

float CalculateQuadraticAttenuation(float distance, float constantAtt, float linearAtt, float quadraticAtt)
{
    return saturate(1.0f / (constantAtt + linearAtt * distance + quadraticAtt * (distance * distance)));
}

float4 CalculateBlinnPhongLighting(Light light, Material mat, float4 colour, float3 lightDirection, float3 normal, float3 toEyeVector)
{
    normal = normalize(normal);
    lightDirection = normalize(lightDirection);
    
    float4 ambientLighting = light.Ambient;
    
    float diffuse = max(dot(normalize(lightDirection), normalize(normal)), 0.0f);
    float4 diffuseLighting = diffuse * light.Diffuse;
    
    toEyeVector = normalize(toEyeVector);
    float3 reflectDir = reflect(-lightDirection, normal);
    
    float specularStrength = 0.5f;
    float specular = pow(max(dot(toEyeVector, reflectDir), 0.0f), 32.0f);
    float4 specularLighting = specularStrength * specular * light.Specular;
    
    return (ambientLighting + diffuseLighting + specularLighting) * (mat.BaseColour * colour);
}

float4 CalculateGouraudShading(Light light, Material mat, float4 colour, float3 lightDirection, float3 normal, float3 toEyeVector)
{
    return CalculateBlinnPhongLighting(light, mat, colour, lightDirection, normal, toEyeVector);
}

float4 CalculateDirectionalLight(Light light, Material mat, float4 colour, float3 normal, float3 worldPosition)
{        
    float3 toEye = normalize(CameraPositionW.xyz - worldPosition);
    return CalculateBlinnPhongLighting(light, mat, colour, normalize(light.Direction.xyz), normalize(normal), normalize(toEye));
}

float4 CalculatePointLight(Light light, Material mat, float4 colour, float3 normal, float3 position)
{
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 CalculateSpotLight(Light light, Material mat, float4 colour, float3 normal, float3 position)
{
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

#endif //__LIGHTING_HLSL__