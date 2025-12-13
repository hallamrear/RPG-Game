#include "pch.h"
#include "Material.h"

Material::Material()
{
	BaseColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Metalness = 0.0f;
	Roughness = 0.0f;
}

Material::~Material()
{
	BaseColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Metalness = 0.0f;
	Roughness = 0.0f;
}