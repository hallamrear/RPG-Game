#include "pch.h"
#include "ColourOnlyVertex.h"
#include <d3d12.h>

ColourOnlyVertex::ColourOnlyVertex()
{
	Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	Colour = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

ColourOnlyVertex::ColourOnlyVertex(const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT4& _colour)
{
	Position = _position;
	Colour = _colour;
}

ColourOnlyVertex::~ColourOnlyVertex()
{
	Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	Colour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
}

const UINT ColourOnlyVertex::GetStride()
{
	return sizeof(ColourOnlyVertex);
}

const UINT ColourOnlyVertex::GetOffset()
{
	return 0;
}

void ColourOnlyVertex::GetElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC>& vectorToFill)
{
	vectorToFill.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ColourOnlyVertex, Position), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	vectorToFill.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(ColourOnlyVertex, Colour), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
}