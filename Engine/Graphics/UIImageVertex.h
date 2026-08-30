#pragma once

#pragma once
#include <DirectXMath.h>

struct D3D12_INPUT_ELEMENT_DESC;

struct UIImageVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 UV;

	UIImageVertex();
	UIImageVertex(const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT2& _uv);
	~UIImageVertex();

	static const UINT GetStride();
	static const UINT GetOffset();
	static void GetElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC>& vectorToFill);
};

