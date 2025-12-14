#pragma once
#include <DirectXMath.h>

class Light
{
public:
	enum LIGHT_TYPE : int
	{
		DIRECTIONAL = LIGHT_DIRECTIONAL,
		POINT = LIGHT_POINT,
		SPOT = LIGHT_SPOT
	};

	LIGHT_TYPE Type;
	int Enabled;
	float InnerCutoff;
	float OuterCutoff;
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Direction;
	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT4 Attenuation;
	DirectX::XMFLOAT4 Strength;
	DirectX::XMFLOAT4 Padding[8];

	Light();
	~Light();
};