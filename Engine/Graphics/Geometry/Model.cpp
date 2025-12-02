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
void Model::TestRender(Renderer& renderer)
{
	Mesh* mesh = m_Meshes[0];
	renderer.GetCommandList()->IASetVertexBuffers(0, 1, &mesh->GetVertexBufferView());
	renderer.GetCommandList()->IASetIndexBuffer(&mesh->GetIndexBufferView());
	renderer.GetCommandList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderer.GetCommandList()->DrawIndexedInstanced(36, 1, 0, 0, 0);
}
