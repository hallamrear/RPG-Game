#include "pch.h"
#include "Model.h"
#include <Graphics/Texturing/Texture.h>

Mesh* Model::CreateNewMesh()
{
	Mesh* mesh = new Mesh();
	m_Meshes.push_back(mesh);
	return mesh;
}

Model::Model()
{
	m_Name = "Model not loaded.";
	m_Meshes = std::vector<Mesh*>();
	m_Textures = std::vector<Texture*>();
}

Model::~Model()
{
	m_Name = "MODEL DESTRUCTOR CALLED.";
	m_Meshes.clear();
	m_Textures.clear();
}

#include <Graphics/Renderer.h>
void Model::Render(Renderer& renderer) const
{
	if (m_Textures.size() > 0)
	{
		renderer.GetCommandList()->SetGraphicsRootDescriptorTable(3, m_Textures[0]->GetSRVHandle());
	}

	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		m_Meshes[i]->Render(renderer);
	}
}
