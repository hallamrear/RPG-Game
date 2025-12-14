#include "pch.h"
#include "World.h"
#include <World/Entity.h>

World::World()
{
	m_Entities = std::vector<Entity*>();
}

World::~World()
{
	for (size_t i = 0; i < m_Entities.size(); i++)
	{
		delete m_Entities[i];
		m_Entities[i] = nullptr;
	}

	m_Entities.clear();
}

void World::Update(const float& deltaTime)
{
	for (size_t i = 0; i < m_Entities.size(); i++)
	{
		m_Entities[i]->Update(deltaTime);
	}
}

void World::Render(Renderer& renderer, Model& model) const
{
	for (size_t i = 0; i < m_Entities.size(); i++)
	{
		m_Entities[i]->Render(renderer, model);
	}
}