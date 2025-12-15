#include "pch.h"
#include "Entity.h"
#include <Graphics/Renderer.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/Geometry/Model.h>
#include <System/GeometryLoader.h>

static int s_EntityCounter = 0;

Entity::Entity()
{
	m_ID = s_EntityCounter;
	s_EntityCounter++;
	m_Parent = nullptr;
	m_Model = nullptr;
	m_LocalMatrix = DirectX::XMFLOAT4X4();
	DirectX::XMStoreFloat4x4(&m_LocalMatrix, DirectX::XMMatrixIdentity());
	m_Material = Material(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, 1.0f);
}

Entity::~Entity()
{
	m_ID = -1;
	m_Parent = nullptr;
	m_Model = nullptr;
	m_LocalMatrix = DirectX::XMFLOAT4X4();
	DirectX::XMStoreFloat4x4(&m_LocalMatrix, DirectX::XMMatrixIdentity());
	m_Material = Material(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0.0f);
}

void Entity::SetParent(Entity* parent)
{
	m_Parent = parent;
}

const std::string& Entity::GetName() const
{
	return m_Name;
}

void Entity::SetName(const std::string& name)
{
	m_Name = name;
}

void Entity::SetModel(const Model* model)
{
	m_Model = model;
}

const DirectX::XMFLOAT4X4& Entity::GetLocalMatrix() const
{
	return m_LocalMatrix;
}

DirectX::XMFLOAT4X4& Entity::GetLocalMatrix()
{
	return m_LocalMatrix;
}

void Entity::SetLocalMatrix(const DirectX::XMFLOAT4X4& localMatrix)
{
	m_LocalMatrix = localMatrix;
}

Material& Entity::GetMaterial()
{
	return m_Material;
}

const Material& Entity::GetMaterial() const
{
	return m_Material;
}

void Entity::SetMaterial(const Material& material)
{
	m_Material = material;
}

const int& Entity::GetID() const
{
	return m_ID;
}

void Entity::Update(const float& deltaTime) 
{
	m_Material.BaseColour.x = (float)rand() / RAND_MAX;
	m_Material.BaseColour.y = (float)rand() / RAND_MAX;
	m_Material.BaseColour.z = (float)rand() / RAND_MAX;
}

void Entity::Render(Renderer& renderer) const
{
	if (m_Model != nullptr)
	{
		DirectX::XMFLOAT4X4 parentMatrix = DirectX::XMFLOAT4X4();
		DirectX::XMStoreFloat4x4(&parentMatrix, DirectX::XMMatrixIdentity());

		if (m_Parent != nullptr)
		{
			parentMatrix = m_Parent->m_LocalMatrix;
		}

		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&parentMatrix) * DirectX::XMLoadFloat4x4(&m_LocalMatrix)));

		renderer.UpdateMaterialBuffer(m_Material);
		renderer.UpdateWorldMatrix(worldMatrix);

		m_Model->Render(renderer);
	}
}