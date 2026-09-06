#include "Structures.hlsli"
#include "Lighting.hlsli"

VS_STANDARD_VERTEX_OUTPUT main(VS_STANDARD_VERTEX_INPUT input)
{
    VS_STANDARD_VERTEX_OUTPUT output = (VS_STANDARD_VERTEX_OUTPUT)0;
    output.PositionW = mul(float4(input.Position, 1.0f), World);
    output.Position = output.PositionW;
    output.Position = mul(output.Position, ViewProjection);
    output.Normal = normalize(input.Normal);
    output.NormalW = mul(float4(input.Normal, 1.0f), transpose(World)).xyz;
    output.NormalW = normalize(output.NormalW);
    output.Tangent = normalize(input.Tangent);
    output.TangentW = mul(float4(input.Tangent, 1.0f), World).xyz;
    output.TangentW = normalize(output.TangentW);
    output.UV = float2(input.UV.x, input.UV.y);
    return output;
}