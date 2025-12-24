#include "pch.h"
#include "Light.h"

Light::Light()
{
	Type = LIGHT_TYPE::DIRECTIONAL;
	Enabled = 0;
	InnerCutoff = 0.0f;
	OuterCutoff = 0.0f;
	Position = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Direction = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Diffuse = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Specular = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	SpecularPower = 0.0f;
	Attenuation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Strength = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = {DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)};
	}
}

Light::~Light()
{
	Type = LIGHT_TYPE::DIRECTIONAL;
	Enabled = 0;
	InnerCutoff = 0.0f;
	OuterCutoff = 0.0f;
	Position = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Direction = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Diffuse = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Specular = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	SpecularPower = 0.0f; 
	Attenuation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	Strength = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < _countof(Padding); i++)
	{
		Padding[i] = { DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) };
	}
}