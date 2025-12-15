#ifndef __LIGHTING_HLSL__
#define __LIGHTING_HLSL__

#include "Structures.hlsli"

float CalculateQuadraticAttenuation(float distance, float constantAtt, float linearAtt, float quadraticAtt)
{
    return saturate(1.0f / (constantAtt + linearAtt * distance + quadraticAtt * (distance * distance)));
}

float3 CalculateSchlickFresnel(float3 R0, float3 normal, float3 lightVector)
{
    const float cosTheta = saturate(dot(normalize(normal), normalize(lightVector)));
    const float F0 = 1.0f - cosTheta;
    return R0 + (1.0f - R0) * pow(F0, 5.0f);
}

float4 CalculateBlinnPhongLighting(Material mat, float3 lightStrength, float3 lightVector, float3 normal, float3 toEyeVector)
{
    const float shininess = 1.0f - mat.Roughness;
    const float m = shininess * 256.0f;
    const float3 halfVec = normalize(toEyeVector + lightVector);
    
    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    
    const float3 r0 = float3(0.04f, 0.04f, 0.04f);
    float3 fresnelFactor = CalculateSchlickFresnel(r0, normal, lightVector);
    
    float3 specAlbedo = fresnelFactor * roughnessFactor;
    //LDR Scaling
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);
    
    return float4((mat.BaseColour.rgb + specAlbedo) * lightStrength, 1.0f);
}

float4 CalculateDirectionalLight(Light light, Material mat, float3 normal, float3 toEye)
{
    float4 output = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVector = -light.Direction.xyz;
    float ndotl = max(dot(normalize(lightVector), normalize(normal)), 0.0f);
    float3 lightStength = light.Strength.xyz * ndotl;
    
    return CalculateBlinnPhongLighting(mat, lightStength, normalize(lightVector), normalize(normal), normalize(toEye));
}

float4 CalculatePointLight(Light light, Material mat, float3 normal, float3 toEye, float3 position)
{
    float4 output = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVector = light.Position.xyz - position;
    float distance = length(lightVector);
    
    float ndotl = max(dot(normalize(lightVector), normalize(normal)), 0.0f);
    float attenuation = CalculateQuadraticAttenuation(distance, light.Attenuation.x, light.Attenuation.y, light.Attenuation.z);
    float3 lightStength = light.Strength.xyz * ndotl * attenuation;
   
    return CalculateBlinnPhongLighting(mat, lightStength, normalize(lightVector), normalize(normal), normalize(toEye));
}

float4 CalculateSpotLight(Light light, Material mat, float3 normal, float3 toEye, float3 position)
{
    float4 output = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVector = light.Position.xyz - position;
    float distance = length(lightVector);
    
    float ndotl = max(dot(normalize(lightVector), normalize(normal)), 0.0f);
    float attenuation = CalculateQuadraticAttenuation(distance, light.Attenuation.x, light.Attenuation.y, light.Attenuation.z);
    
    float theta = dot(normalize(-lightVector), normalize(-light.Direction.xyz));
    float epsilon = light.InnerCutoff - light.OuterCutoff;
    float intensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0f, 1.0f);
    
    float3 lightStength = light.Strength.xyz * ndotl * attenuation * intensity;
   
    return CalculateBlinnPhongLighting(mat, lightStength, normalize(lightVector), normalize(normal), normalize(toEye));
}

#endif //__LIGHTING_HLSL__