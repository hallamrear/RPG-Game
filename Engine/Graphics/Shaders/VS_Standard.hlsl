#include "Structures.hlsli"

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
    output.UV = float2(1.0f - input.UV.x, input.UV.y);
    return output;
}