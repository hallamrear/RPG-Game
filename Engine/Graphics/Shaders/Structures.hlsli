#ifndef __STRUCTURES_HLSL__
#define __STRUCTURES_HLSL__

#include "../../Defines.h"

cbuffer PerFrameConstantBuffer : register(b0)
{
    float4 CameraPositionW;
    float4 CameraDirectionW;
    float4 Padding[14];
};

struct Material
{
    float4 BaseColour;
    float Metalness;
    float Roughness;
    float Padding[2];
};

cbuffer PerObjectConstantBuffer : register(b1)
{
	/* 64b */ float4x4 World;
    /* 64b */ float4x4 ViewProjection;
};

cbuffer PerObjectTextureBuffer : register(b2)
{
	/* 32b */ Material MaterialData;
	/* 4b * MAX_TEXTURES_PER_SHADER */ uint TextureIndex[MAX_TEXTURES_PER_SHADER];
};

struct Light
{
    int Type;
    int Enabled;
    float InnerCutoff;
    float OuterCutoff;
    float4 Position;
    float4 Direction;
    float4 Diffuse;
    float3 Specular;
    float SpecularPower;
    float4 Attenuation;
    float4 Strength;
    float4 Padding[9];
};

cbuffer LightingBuffer : register(b3)
{
    Light LightData[MAX_LIGHT_COUNT];
};

struct VS_STANDARD_VERTEX_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
};

struct VS_STANDARD_VERTEX_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 PositionW : POSITION;
    float3 Normal : NORMAL0;
    float3 NormalW : NORMAL1;
    float3 Tangent : TANGENT0;
    float3 TangentW : TANGENT1;
    float2 UV : TEXCOORD;
};

struct VS_COLOUR_ONLY_INPUT
{
    float3 Position : POSITION;
    float4 Colour : COLOR;
};

struct VS_COLOUR_ONLY_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 PositionW : POSITION;
    float4 Colour : COLOR;
};

struct VS_UI_IMAGE_VERTEX_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VS_UI_IMAGE_VERTEX_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

#endif //__STRUCTURES_HLSL__