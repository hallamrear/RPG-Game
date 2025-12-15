#pragma once
#include <DirectXMath.h>

class Material
{
public:
	DirectX::XMFLOAT4 BaseColour;
	float Metalness;
	float Roughness;
	float Padding[2];

	Material();
	Material(const DirectX::XMFLOAT4& baseColour, const float& roughness, const float& metalness);
	~Material();
};

