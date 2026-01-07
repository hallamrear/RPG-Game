#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif // !_USE_MATH_DEFINES

#include <math.h>
#include <DirectXMath.h>

#define DEGREES_TO_RADIANS (M_PI * 180.0f)
#define RADIANS_TO_DEGREES (180.0f / M_PI)

constexpr DirectX::XMFLOAT3 BASIS_RIGHT_VECTOR = { 1.0f, 0.0f, 0.0f };
constexpr DirectX::XMFLOAT3 BASIS_UP_VECTOR = { 0.0f, 1.0f, 0.0f };
constexpr DirectX::XMFLOAT3 BASIS_FORWARD_VECTOR = { 0.0f, 0.0f, 1.0f };