#pragma once
#include <DirectXMath.h>

struct D3D12_INPUT_ELEMENT_DESC;

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT2 UV;

	Vertex();
	Vertex(const DirectX::XMFLOAT3& _position, const DirectX::XMFLOAT3& _normal, const DirectX::XMFLOAT3& _tangent, const DirectX::XMFLOAT2& _uv);
	~Vertex();

	static const UINT GetStride();
	static const UINT GetOffset();
	static void GetElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC>& vectorToFill);
};

