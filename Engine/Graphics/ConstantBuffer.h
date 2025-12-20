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
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
	DirectX::XMFLOAT4 CameraPosition;
	DirectX::XMFLOAT4 CameraDirection;
	DirectX::XMFLOAT4 Padding[6];

	ConstantBuffer();
	~ConstantBuffer();
};

struct PushConstants
{
	/* 64b */ DirectX::XMFLOAT4X4 World;
	/* 32b */ Material MaterialData;
};

class LightBuffer
{
public:
	Light LightData[MAX_LIGHT_COUNT];
};