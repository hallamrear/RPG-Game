#include "Structures.hlsli"

VS_UI_IMAGE_VERTEX_OUTPUT main(VS_UI_IMAGE_VERTEX_INPUT input)
{
    VS_UI_IMAGE_VERTEX_OUTPUT output = (VS_UI_IMAGE_VERTEX_OUTPUT) 0;
    output.Position = float4(input.Position.xyz, 1.0f);
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, Projection);
    output.UV = input.UV;
    return output;
}