#include "pch.h"
#include "Camera.h"
#include <System/Debug.h>


Camera::Camera() : EventHandler()
{
	DirectX::XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_RotationMatrix, DirectX::XMMatrixIdentity());
	m_LeftVector = { 1.0f, 0.0f, 0.0f };
	m_UpVector = { 0.0f, 1.0f, 0.0f };
	m_ForwardVector = { 0.0f, 0.0f, 1.0f };
	m_Translation = { 0.0f, 0.0f, 0.0f };
	UpdateTransformMatrix();
}

Camera::~Camera()
{

}

void Camera::UpdateTransformMatrix()
{
	DirectX::XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMMatrixIdentity() * DirectX::XMLoadFloat4x4(&m_RotationMatrix) * DirectX::XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z));

	m_LeftVector.x = m_WorldMatrix.m[0][0];
	m_LeftVector.y = m_WorldMatrix.m[0][1];
	m_LeftVector.z = m_WorldMatrix.m[0][2];

	m_UpVector.x = m_WorldMatrix.m[1][0];
	m_UpVector.y = m_WorldMatrix.m[1][1];
	m_UpVector.z = m_WorldMatrix.m[1][2];

	m_ForwardVector.x = m_WorldMatrix.m[2][0];
	m_ForwardVector.y = m_WorldMatrix.m[2][1];
	m_ForwardVector.z = m_WorldMatrix.m[2][2];
}

void Camera::Move(const DirectX::XMFLOAT3& movement)
{
	m_Translation.x += movement.x;
	m_Translation.y += movement.y;
	m_Translation.z += movement.z;
	UpdateTransformMatrix();
}

void Camera::RotateEuler(const DirectX::XMFLOAT3& rotationEuler)
{
	DirectX::XMStoreFloat4x4(&m_RotationMatrix, DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationRollPitchYaw(rotationEuler.x, rotationEuler.y, rotationEuler.z), DirectX::XMLoadFloat4x4(&m_RotationMatrix)));
	UpdateTransformMatrix();
}

void Camera::RotateQuat(const DirectX::XMFLOAT4& rotationQuat)
{
	DirectX::XMStoreFloat4x4(&m_RotationMatrix, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&m_RotationMatrix), DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotationQuat))));
	UpdateTransformMatrix();
}

bool Camera::HandleEvent(const float& deltaTime, const Event& event)
{
	switch (event.GetType())
	{
	case CAMERA_EVENT:

		break;

	case INPUT_EVENT:
	{
		float step = (5000.0f * deltaTime);
		float rotation = 1000.0f * deltaTime;

		switch (event.Data.Input.Key)
		{
			/* E Key */
		case 0x45: RotateEuler({ 0.0f, +1.0F * rotation, 0.0f }); break;
			/* Q Key */
		case 0x51: RotateEuler({ 0.0f, -1.0F * rotation, 0.0f }); break;

			/* R Key */
		case 0x52: RotateEuler({ 0.0f, +1.0F * rotation, 0.0f }); break;
			/* F Key */
		case 0x46: RotateEuler({ m_LeftVector.x * rotation, m_LeftVector.y * rotation, m_LeftVector.z * rotation }); break;

			/* Up Arrow */
		case VK_UP: Move({ m_ForwardVector.x * step, m_ForwardVector.y * step, m_ForwardVector.z * step }); break;
			/* Down Arrow */
		case VK_DOWN:  Move({ 0.0f, 0.0f, -step }); break;
			/* Left Arrow */
		case VK_LEFT:  Move({ -step, 0.0f, 0.0f }); break;
			/* Right Arrow */
		case VK_RIGHT: Move({ step, 0.0f, 0.0f }); break;
			/* Space Bar */
		case VK_SPACE: Move({ m_UpVector.x * step, m_UpVector.y * step, m_UpVector.z * step }); break;
			/* Left Shift */
		case VK_LSHIFT: Move({ 0.0f, -step, 0.0f }); break;

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
	DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_WorldMatrix))));
	return view;
}