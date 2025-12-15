#pragma once
#include "Mesh.h"

class Texture;
class Renderer;
class Material;

class Model
{
private:
	friend class GeometryLoader;
	std::string m_Name;
	std::vector<Material*> m_Materials;
	std::vector<Mesh*> m_Meshes;
	std::vector<Texture*> m_Textures;

protected:
	Mesh* CreateNewMesh();

public:
	Model();
	~Model();

	void Render(Renderer& renderer) const;
};

