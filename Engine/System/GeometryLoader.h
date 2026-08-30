#pragma once
#include <DirectXMath.h>
#include <Graphics/Renderer.h>

struct Vertex;
struct ColourOnlyVertex;

namespace tinygltf
{
	class Model;
	struct Primitive;
	struct Mesh;
}

class Model;
class Mesh;
class Renderer;
class Material;

class GeometryLoader
{
private:
	friend class SceneLoader;

	static bool CreateModelFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel, const std::string& parentPath);
	static bool LoadGeometryFromGLTFMesh(Renderer& renderer, Model& model, tinygltf::Model& gltfModel, const int& meshIndex);
	static bool GetVertexDataFromGLTFPrimitive(std::vector<Vertex>& vertices, tinygltf::Model& model, tinygltf::Primitive& primitive, DirectX::XMFLOAT3& maxPosition, DirectX::XMFLOAT3& minPosition);
	static bool GetIndexDataFromGLTFPrimitive(std::vector<uint16_t>& indices, const tinygltf::Model& model, const tinygltf::Primitive& gltfMesh);
	static bool LoadTexturesFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel, const std::string& parentPath);
	static bool GetElementDataFromGLTFBuffer(const std::string& attributeName, byte*& data, tinygltf::Model& model, tinygltf::Primitive& primitive);

public:
	template<class V, class I>
	static Mesh* CreateMeshFromData(Renderer& renderer, Model& model, std::vector<V>& vertices, std::vector<I>& indices, const D3D12_PRIMITIVE_TOPOLOGY& topology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
	static bool Load(Renderer& renderer, Model& model, const std::string& path);
	static void Destroy(Model& model);
};

#include <Graphics/Renderer.h>
#include <Graphics/Geometry/Model.h>
#include <System/Debug.h>

template<class V, class I>
Mesh* GeometryLoader::CreateMeshFromData(Renderer& renderer, Model& model, std::vector<V>& vertices, std::vector<I>& indices, const D3D12_PRIMITIVE_TOPOLOGY& topology)
{
    ID3D12Resource* vertexBuffer = nullptr;
    ID3D12Resource* vbUploader = nullptr;
    ID3D12Resource* indexBuffer = nullptr;
    ID3D12Resource* ibUploader = nullptr;

    size_t vbSize = sizeof(V) * vertices.size();

    HRESULT result = renderer.CreateDefaultBuffer(vertexBuffer, vbUploader, (const void*)vertices.data(), vbSize);

    if (FAILED(result) || vbUploader == nullptr)
    {
        Debug::LogSevere("Failed to create vertex buffer for gltf model.\n");

        if (vertexBuffer != nullptr)
        {
            vertexBuffer->Release();
            vertexBuffer = nullptr;
        }

        if (vbUploader != nullptr)
        {
            vbUploader->Release();
            vbUploader = nullptr;
        }

        return nullptr;
    }

    CD3DX12_RESOURCE_BARRIER vbTransition = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    renderer.GetCommandList()->ResourceBarrier(1, &vbTransition);

    vbUploader->SetName(L"VB Uploader");

    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbv.SizeInBytes = vbSize;
    vbv.StrideInBytes = sizeof(V);

    size_t ibSize = sizeof(I) * indices.size();

    D3D12_INDEX_BUFFER_VIEW ibv{};

    bool usesIndexBuffer = ibSize > 0;

    if (usesIndexBuffer)
    {
        result = renderer.CreateDefaultBuffer(indexBuffer, ibUploader, (const void*)indices.data(), ibSize);

        if (FAILED(result) || ibUploader == nullptr)
        {
            Debug::LogSevere("Failed to create vertex buffer for gltf model.\n");

            if (indexBuffer != nullptr)
            {
                indexBuffer->Release();
                indexBuffer = nullptr;
            }

            if (ibUploader != nullptr)
            {
                ibUploader->Release();
                ibUploader = nullptr;
            }

            return nullptr;
        }

        CD3DX12_RESOURCE_BARRIER ibTransition = CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        renderer.GetCommandList()->ResourceBarrier(1, &ibTransition);

        ibUploader->SetName(L"IB Uploader");

        ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        ibv.Format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
        ibv.SizeInBytes = ibSize;
    }

    if (vbSize <= 0 && ibSize <= 0)
    {
        Debug::LogSevere("Failed to create mesh with given data.\n");
        return nullptr;
    }

    Mesh* mesh = model.CreateNewMesh();
    mesh->m_UsesIndexBuffer = usesIndexBuffer;
    mesh->m_VertexBuffer = vertexBuffer;
    mesh->m_VertexBufferView = vbv;
    mesh->m_VertexCount = vertices.size();

    if (mesh->m_UsesIndexBuffer)
    {
        mesh->m_IndexBuffer = indexBuffer;
        mesh->m_IndexBufferView = ibv;
        mesh->m_IndexCount = indices.size();
    }

    if (vbUploader != nullptr)
    {
        vbUploader->Release();
        vbUploader = nullptr;
    }

    if (ibUploader != nullptr)
    {
        ibUploader->Release();
        ibUploader = nullptr;
    }

    if (topology != D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
    {
        mesh->m_TopologyType = topology;
    }

    return mesh;
};