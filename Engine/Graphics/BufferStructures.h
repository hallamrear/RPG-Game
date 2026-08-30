#pragma once
#include <DirectXMath.h>
#include <Defines.h>
#include <Graphics/Lighting/Light.h>
#include <Graphics/Texturing/Material.h>

/// <summary>
/// Constant buffers have to be 256-byte aligned in DX12.
/// </summary>

class ConstantBuffer
{
public:
	DirectX::XMFLOAT4 CameraPosition;
	DirectX::XMFLOAT4 CameraDirection;
	DirectX::XMFLOAT4 Padding[14];

	ConstantBuffer();
	~ConstantBuffer();
};

struct PushConstants
{
	/* 64b */ DirectX::XMFLOAT4X4 World;
	/* 64b */ DirectX::XMFLOAT4X4 View;
	/* 64b */ DirectX::XMFLOAT4X4 Projection;
	UINT32 TextureIndex;
	///* 32b */ Material MaterialData;
	///* 4b * MAX_TEXTURES_PER_SHADER */ UINT32 TextureSlotEnabled[MAX_TEXTURES_PER_SHADER];

	PushConstants();
	~PushConstants();
};

class LightBuffer
{
public:
	Light LightData[MAX_LIGHT_COUNT];
};