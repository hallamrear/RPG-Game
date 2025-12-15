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
	m_WorldMatrix = DirectX::XMFLOAT4X4();
	DirectX::XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMMatrixIdentity());
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

Entity* Entity::GetParent() const
{
	return m_Parent;
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

const DirectX::XMFLOAT4X4& Entity::GetWorldMatrix()
{
	if (m_Parent != nullptr)
	{
		DirectX::XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMLoadFloat4x4(&m_Parent->GetWorldMatrix()) * DirectX::XMLoadFloat4x4(&GetLocalMatrix()));
		return m_WorldMatrix;
	}

	m_WorldMatrix = m_LocalMatrix;
	return m_WorldMatrix;
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
	GetWorldMatrix();
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

}

void Entity::Render(Renderer& renderer)
{
	if (m_Model != nullptr)
	{
		DirectX::XMFLOAT4X4 matrix;
		DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&GetWorldMatrix())));
		renderer.UpdateWorldMatrix(matrix);
		renderer.UpdateMaterialBuffer(m_Material);
		m_Model->Render(renderer);
	}
}