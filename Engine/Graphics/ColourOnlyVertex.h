#pragma once
#include <DirectXMath.h>

struct D3D12_INPUT_ELEMENT_DESC;

struct ColourOnlyVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Colour;

	ColourOnlyVertex();
	ColourOnlyVertex(const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT4& _colour);
	~ColourOnlyVertex();

	static const UINT GetStride();
	static const UINT GetOffset();
	static void GetElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC>& vectorToFill);
};
