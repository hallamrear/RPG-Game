#pragma once
#include <DirectXMath.h>

/// <summary>
/// Constant buffer has to be 256-byte aligned in DX12.
/// </summary>
class ConstantBuffer
{
public:
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
	DirectX::XMFLOAT4X4 Padding;

	ConstantBuffer();
	~ConstantBuffer();
};

