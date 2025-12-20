#include "Structures.hlsli"
#include "Lighting.hlsli"

VS_STANDARD_VERTEX_OUTPUT main(VS_STANDARD_VERTEX_INPUT input)
{
    VS_STANDARD_VERTEX_OUTPUT output = (VS_STANDARD_VERTEX_OUTPUT)0;
    output.PositionW = mul(float4(input.Position, 1.0f), World);
    output.Position = output.PositionW;
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    output.Normal = input.Normal;
    output.NormalW = mul(float4(input.Normal, 1.0f), World).xyz;
    output.Tangent = input.Tangent;
    output.TangentW = mul(float4(input.Tangent, 1.0f), World).xyz;
    output.UV = float2(input.UV.x, input.UV.y);
    
    float4 lightColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        Light light = LightData[i];
        
        if (light.Enabled == 0)
            continue;
        
        lightColour += CalculateGouraudShading(light, MaterialData, MaterialData.BaseColour, light.Direction.rgb, output.NormalW, output.PositionW.xyz);
    }
    
    output.Colour = lightColour;
    
    return output;
}