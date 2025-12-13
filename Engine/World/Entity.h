#pragma once
#include <Graphics/DX12Includes.h>

class Model;
class Renderer;

class Entity
{
private:
	Entity* m_Parent;
	DirectX::XMFLOAT3 m_LocalMatrix;

	Model* m_Model;

public:
	Entity();
	~Entity();

	virtual void Update(const float& deltaTime);
	virtual void Render(const Renderer& renderer) const;
};

