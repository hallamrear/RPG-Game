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
	const Model* m_Model;
	Material m_Material;
	std::vector<Entity*> m_Children;

public:
	Entity();
	~Entity();

	void SetParent(Entity* parent);
	Entity* GetParent() const;

	int GetChildCount() const;
	Entity* GetChild(const int& index) const;
	std::vector<Entity*>& GetChildren();

	const std::string& GetName() const;
	void SetName(const std::string& name);

	void SetModel(Model* model);

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

