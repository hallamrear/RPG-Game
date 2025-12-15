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
}

Entity::~Entity()
{
	m_ID = -1;
	m_Parent = nullptr;
	m_Model = nullptr;
	m_LocalMatrix = DirectX::XMFLOAT4X4();
	DirectX::XMStoreFloat4x4(&m_LocalMatrix, DirectX::XMMatrixIdentity());
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

void Entity::TEMP_setmodel(Renderer& renderer)
{
	m_Model = new Model();
	GeometryLoader::Load(renderer, *m_Model, "Resources/OSRS_Model.gltf");
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

const int& Entity::GetID() const
{
	return m_ID;
}

void Entity::Update(const float& deltaTime) 
{

}

void Entity::Render(Renderer& renderer, Model& model) const
{
	DirectX::XMFLOAT4X4 parentMatrix = DirectX::XMFLOAT4X4();
	DirectX::XMStoreFloat4x4(&parentMatrix, DirectX::XMMatrixIdentity());

	if (m_Parent != nullptr)
	{
		parentMatrix = m_Parent->m_LocalMatrix;
	}

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMLoadFloat4x4(&parentMatrix) * DirectX::XMLoadFloat4x4(&m_LocalMatrix));

	renderer.UpdateWorldMatrix(worldMatrix);

	model.TestRender(renderer);
}