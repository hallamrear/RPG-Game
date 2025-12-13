#pragma once
#include <DirectXMath.h>

class Material
{
public:
	DirectX::XMFLOAT4 BaseColour;
	float Metalness;
	float Roughness;

	Material();
	~Material();

private:
	float Padding[58];
};

