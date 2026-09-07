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

struct PerObjectMatrixData
{
	/* 64b */ DirectX::XMFLOAT4X4 World;
	/* 64b */ DirectX::XMFLOAT4X4 ViewProjection;
	
	PerObjectMatrixData();
	~PerObjectMatrixData();
};

struct PerObjectTextureData
{
	/* 32b */ Material MaterialData;
	/* 32b * MAX_TEXTURES_PER_SHADER */ UINT32 TextureSlotIDs[MAX_TEXTURES_PER_SHADER];
	UINT32 Padding[3];

	PerObjectTextureData();
	~PerObjectTextureData();

	void Reset();
};

class LightBuffer
{
public:
	Light LightData[MAX_LIGHT_COUNT];
};