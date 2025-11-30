#include "pch.h"
#include "Model.h"
#include <Graphics/Texturing/Texture.h>

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