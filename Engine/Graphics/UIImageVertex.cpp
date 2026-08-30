#include "pch.h"
#include "UIImageVertex.h"
#include <d3d12.h>

UIImageVertex::UIImageVertex()
{
	Position = { 0.0f, 0.0f, 0.0f };
	UV = { 0.0f, 0.0f };
}

UIImageVertex::UIImageVertex(const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT2& _uv)
{
	Position = _position;
	UV = _uv;
}

UIImageVertex::~UIImageVertex()
{
	Position = { 0.0f, 0.0f, 0.0f };
	UV = { 0.0f, 0.0f };
}

const UINT UIImageVertex::GetStride()
{
	return sizeof(UIImageVertex);
}

const UINT UIImageVertex::GetOffset()
{
	return 0;
}

void UIImageVertex::GetElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC>& vectorToFill)
{
	vectorToFill.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(UIImageVertex, Position), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	vectorToFill.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(UIImageVertex, UV), D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
}