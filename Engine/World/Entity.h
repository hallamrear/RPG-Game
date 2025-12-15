#pragma once
#include <Graphics/DX12Includes.h>
#include <Graphics/Texturing/Material.h>

class Model;
class Renderer;

class Entity
{
private:
	int m_ID;
	std::string m_Name;
	Entity* m_Parent;
	DirectX::XMFLOAT4X4 m_LocalMatrix;
	DirectX::XMFLOAT4X4 m_WorldMatrix;
	Material m_Material;
	const Model* m_Model;

public:
	Entity();
	~Entity();

	void SetParent(Entity* parent);
	Entity* GetParent() const;

	const std::string& GetName() const;
	void SetName(const std::string& name);

	void SetModel(const Model* model);

	const DirectX::XMFLOAT4X4& GetWorldMatrix();
	const DirectX::XMFLOAT4X4& GetLocalMatrix() const;
	DirectX::XMFLOAT4X4& GetLocalMatrix();
	void SetLocalMatrix(const DirectX::XMFLOAT4X4& localMatrix);

	Material& GetMaterial();
	const Material& GetMaterial() const;
	void SetMaterial(const Material& material);

	const int& GetID() const;
	virtual void Update(const float& deltaTime);
	virtual void Render(Renderer& renderer);
};

