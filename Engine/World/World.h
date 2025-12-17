#pragma once
#include <vector>
#include <unordered_map>

class Entity;
class Model;
class Renderer;

class World
{
private:
	friend class SceneLoader;
	std::unordered_map<std::string, int> m_EntityNameToIDMap;
	std::unordered_map<int, Entity*> m_EntityIDToObjectMap;
	std::vector<Entity*> m_Entities;

public:
	World();
	~World();

	void Update(const float& deltaTime);
	void Render(Renderer& renderer) const;
	void PrintHierarchy(Entity* entity, int& depth);
};