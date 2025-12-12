#pragma once
#include "Mesh.h"

class Texture;
class Renderer;

class Model
{
private:
	friend class GeometryLoader;
	std::string m_Name;
	std::vector<Mesh*> m_Meshes;
	std::vector<Texture*> m_Textures;

protected:
	Mesh* CreateNewMesh();

public:
	Model();
	~Model();

	void TestRender(int index, Renderer& renderer);
};

