#pragma once
#include <System/Events/EventSystem.h>
#include <DirectXMath.h>

class Camera : public EventHandler
{
private:
	DirectX::XMFLOAT3 m_Translation;
	DirectX::XMFLOAT4X4 m_RotationMatrix;
	DirectX::XMFLOAT4X4 m_WorldMatrix;
	void UpdateTransformMatrix();

	DirectX::XMFLOAT3 m_ForwardVector;
	DirectX::XMFLOAT3 m_LeftVector;
	DirectX::XMFLOAT3 m_UpVector;

public:
	Camera();
	~Camera();


	void Move(const DirectX::XMFLOAT3& movement);
	void RotateEuler(const DirectX::XMFLOAT3& rotationEuler);
	void RotateQuat(const DirectX::XMFLOAT4& rotationQuat);

	virtual bool HandleEvent(const float& deltaTime, const Event& event);

	const DirectX::XMFLOAT4X4 GetViewMatrix() const;
};

