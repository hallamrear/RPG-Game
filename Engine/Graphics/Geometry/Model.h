#pragma once

class Mesh;
class Texture;

class Model
{
private:
	friend class GeometryLoader;
	std::string m_Name;
	std::vector<Mesh*> m_Meshes;
	std::vector<Texture*> m_Textures;

protected:

public:
	Model();
	~Model();

};

