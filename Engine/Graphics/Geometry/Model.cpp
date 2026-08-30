#include "pch.h"
#include "Model.h"
#include <Graphics/Renderer.h>
#include <Graphics/Texturing/Material.h>
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
	m_Materials = std::vector<Material*>();
}

Model::~Model()
{
	m_Name = "DESTROYED MODEL";

	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		delete m_Meshes[i];
		m_Meshes[i] = nullptr;
	}
	m_Meshes.clear();

	for (size_t i = 0; i < m_Textures.size(); i++)
	{
		delete m_Textures[i];
		m_Textures[i] = nullptr;
	}
	m_Textures.clear();

	for (size_t i = 0; i < m_Materials.size(); i++)
	{
		delete m_Materials[i];
		m_Materials[i] = nullptr;
	}
	m_Materials.clear();
}

void Model::AddTexture(Texture* texture)
{
	if (texture != nullptr)
	{
		m_Textures.push_back(texture);
	}
}

void Model::Render(Renderer& renderer) const
{
	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		for (size_t t = 0; t < 5; t++)
		{
			renderer.AssignTextureToSlot(t, nullptr);
		}

		for (size_t t = 0; t < 5; t++)
		{
			int index = m_Meshes[i]->GetTextureID();

			if (index >= 0)
			{
				Texture* texture = m_Textures[m_Meshes[i]->GetTextureID()];

				if (texture != nullptr)
				{
					renderer.AssignTextureToSlot(t, texture);
				}
			}
			else
			{
				renderer.AssignTextureToSlot(t, nullptr);
			}
		}

		const Material* material = &Material::GetDefaultMaterial();

		int id = m_Meshes[i]->GetMaterialID();

		if (id > -1 && id < m_Materials.size())
		{
			material = m_Materials[id];
		}

		renderer.UpdateMaterialBuffer(*material);

		m_Meshes[i]->Render(renderer);
	}
}
