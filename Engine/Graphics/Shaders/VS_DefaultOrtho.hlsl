#include "Structures.hlsli"

VS_COLOUR_ONLY_OUTPUT main(VS_COLOUR_ONLY_INPUT input)
{
    VS_COLOUR_ONLY_OUTPUT output = (VS_COLOUR_ONLY_OUTPUT) 0;
    output.Position = float4(input.Position.xyz, 1.0f);
    //output.PositionW = mul(float4(input.Position.xyz, 1.0f), World);
    //output.Position = output.PositionW;
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    output.Colour = input.Colour;
    return output;
}