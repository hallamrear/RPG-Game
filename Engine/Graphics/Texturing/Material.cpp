#include "pch.h"
#include "Material.h"

Material Material::m_DefaultMaterial = Material();

Material::Material()
{
	BaseColour = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Metalness = 1.0f;
	Roughness = 1.0f;

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = 0.0f;
	}
}

Material::Material(const DirectX::XMFLOAT4& baseColour, const float& roughness, const float& metalness)
{
	BaseColour = baseColour;
	Roughness = roughness;
	Metalness = metalness;

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = 0.0f;
	}
}

Material::~Material()
{
	BaseColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Metalness = 0.0f;
	Roughness = 0.0f;

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = 0.0f;
	}
}

const Material& Material::GetDefaultMaterial()
{
	return m_DefaultMaterial;
}
