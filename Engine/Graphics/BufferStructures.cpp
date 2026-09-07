#include "pch.h"
#include "BufferStructures.h"

ConstantBuffer::ConstantBuffer()
{
	CameraPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	CameraDirection = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

ConstantBuffer::~ConstantBuffer()
{
	CameraPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	CameraDirection = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

PerObjectMatrixData::PerObjectMatrixData()
{
	ViewProjection = DirectX::XMFLOAT4X4();
	World = DirectX::XMFLOAT4X4();
}

PerObjectMatrixData::~PerObjectMatrixData()
{
	ViewProjection = DirectX::XMFLOAT4X4();
	World = DirectX::XMFLOAT4X4();
}

PerObjectTextureData::PerObjectTextureData()
{
	memset(&TextureSlotIDs, 0x0, sizeof(TextureSlotIDs[0]) * MAX_TEXTURES_PER_SHADER);
	memset(&MaterialData, 0x0, sizeof(Material));
	MaterialData.BaseColour = { 1.0f, 1.0f, 1.0f, 1.0f };

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = 0xFF;
	}
}

PerObjectTextureData::~PerObjectTextureData()
{
	memset(&TextureSlotIDs, 0x0, sizeof(TextureSlotIDs[0]) * MAX_TEXTURES_PER_SHADER);
	memset(&MaterialData, 0x0, sizeof(Material));

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = 0xFF;
	}
}

void PerObjectTextureData::Reset()
{
	memset(&TextureSlotIDs, 0x0, sizeof(TextureSlotIDs[0]) * MAX_TEXTURES_PER_SHADER);
	memset(&MaterialData, 0x0, sizeof(Material));
	MaterialData.BaseColour = { 1.0f, 1.0f, 1.0f, 1.0f };

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = 0xFF;
	}
}
