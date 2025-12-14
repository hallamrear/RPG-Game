#pragma once
#include <Graphics/DX12Includes.h>

class Model;
class Renderer;

class Entity
{
private:
	int m_ID;
	std::string m_Name;
	Entity* m_Parent;
	DirectX::XMFLOAT4X4 m_LocalMatrix;

	Model* m_Model;

public:
	Entity();
	~Entity();

	void SetParent(Entity* parent);
	const std::string& GetName() const;
	void SetName(const std::string& name);

	void TEMP_setmodel(Renderer& renderer);

	const DirectX::XMFLOAT4X4& GetLocalMatrix() const;
	DirectX::XMFLOAT4X4& GetLocalMatrix();
	void SetLocalMatrix(const DirectX::XMFLOAT4X4& localMatrix);

	const int& GetID() const;
	virtual void Update(const float& deltaTime);
	virtual void Render(Renderer& renderer, Model& model) const;
};

