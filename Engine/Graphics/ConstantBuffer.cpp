#include "pch.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
{
	View = DirectX::XMFLOAT4X4();
	Projection = DirectX::XMFLOAT4X4();
	CameraPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	CameraDirection = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

ConstantBuffer::~ConstantBuffer()
{
	View = DirectX::XMFLOAT4X4();
	Projection = DirectX::XMFLOAT4X4();
	CameraPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	CameraDirection = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}
