#include "pch.h"
#include "World.h"
#include <World/Entity.h>
#include <System/Debug.h>

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

void World::Render(Renderer& renderer) const
{
	for (size_t i = 0; i < m_Entities.size(); i++)
	{
		m_Entities[i]->Render(renderer);
	}
}

void World::PrintHierarchy(Entity* entity, int& depth)
{
	for (size_t i = 0; i < depth; i++)
	{
		Debug::LogMessage("\t");
	}

	Debug::LogMessage("Entity: %s\n", entity->GetName().c_str());

	if (entity->GetChildCount() != 0)
	{
		for (size_t i = 0; i < entity->GetChildCount(); i++)
		{
			int newD = depth + 1;
			PrintHierarchy(entity->GetChild(i), newD);
		}
	}

}