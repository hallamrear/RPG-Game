#include "pch.h"
#include "Camera.h"
#include <System/Debug.h>

Camera::Camera() : EventHandler()
{
	m_Position.x = 0.0f;
	m_Position.y = 10.0f;
	m_Position.z = -10.0f;
}

Camera::~Camera()
{
	m_Position.x = 0.0f;
	m_Position.y = 0.0f;
	m_Position.z = 0.0f;
}

bool Camera::HandleEvent(const float& deltaTime, const Event& event)
{
	switch (event.GetType())
	{
	case CAMERA_EVENT:
		break;

	case INPUT_EVENT:
	{
		float step = (350.0f * deltaTime);

		switch (event.Data.Input.Key)
		{
			/* Up Arrow */
		case VK_UP: m_Position.z += step; break;
			/* Down Arrow */
		case VK_DOWN: m_Position.z -= step; break;
			/* Left Arrow */
		case VK_LEFT: m_Position.x -= step; break;
			/* Right Arrow */
		case VK_RIGHT: m_Position.x += step; break;

		default:
			break;
		}
	}
		break;

	default:
		return false;
	}

	return true;
}

const DirectX::XMFLOAT4X4 Camera::GetViewMatrix() const
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH({ m_Position.x, m_Position.y, m_Position.z }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f })));
	return view;
}