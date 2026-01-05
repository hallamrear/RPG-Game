#pragma once
#include <System/Events/EventSystem.h>
#include <DirectXMath.h>

class Camera : public EventHandler
{
private:
	DirectX::XMFLOAT3 m_Position;

public:
	Camera();
	~Camera();

	virtual bool HandleEvent(const float& deltaTime, const Event& event);

	const DirectX::XMFLOAT4X4 GetViewMatrix() const;
};

