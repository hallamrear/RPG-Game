#include "pch.h"
#include "GeometryLoader.h"
#include <System/Debug.h>
#include <System/FileLoadingIncludes.h>
#include <System/TextureLoader.h>
#include <Graphics/Texturing/Texture.h>
#include <Graphics/Geometry/Mesh.h>
#include <Graphics/Geometry/Model.h>
#include <Graphics/Renderer.h>

bool GeometryLoader::CreateModelFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel)
{
    bool loadedOk = true;

    loadedOk &= LoadGeometryFromGLTF(renderer, model, gltfModel);
    loadedOk &= LoadTexturesFromGLTF(renderer, model, gltfModel);

    return loadedOk;
}

#include <Graphics/ColourOnlyVertex.h>

bool GeometryLoader::LoadGeometryFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel)
{
    ID3D12Resource* vbUploader = nullptr;
    ID3D12Resource* vertexBuffer = nullptr;
    ID3D12Resource* ibUploader = nullptr;
    ID3D12Resource* indexBuffer = nullptr;

    ColourOnlyVertex vertices[] =
    {
        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }
    };

    size_t vbSize = sizeof(ColourOnlyVertex) * 8;

    HRESULT result = renderer.CreateDefaultBuffer(vertexBuffer, vbUploader, (const void*)vertices, vbSize);

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

        return false;
    }

    std::uint16_t indices[] =
    {
        0, 1, 2,
        0, 2, 3,

        4, 6, 5,
        4, 7, 6,

        4, 5, 1,
        4, 1, 0, 

        3, 2, 6,
        3, 6, 7,

        1, 5, 6,
        1, 6, 2,

        4, 0, 3,
        4, 3, 7
    };

    size_t ibSize = sizeof(uint16_t) * 36;

    result = renderer.CreateDefaultBuffer(indexBuffer, ibUploader, (const void*)indices, ibSize);

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

        return false;
    }

    ibUploader->SetName(L"IB Uploader");
    vbUploader->SetName(L"VB Uploader");
    
    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbv.SizeInBytes = vbSize;
    vbv.StrideInBytes = sizeof(ColourOnlyVertex);

    D3D12_INDEX_BUFFER_VIEW ibv{};
    ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    ibv.Format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
    ibv.SizeInBytes = ibSize;

    Mesh* mesh = model.CreateNewMesh();
    mesh->m_VertexBuffer = vertexBuffer;
    mesh->m_VertexBufferView = vbv;
    mesh->m_IndexBuffer = indexBuffer;
    mesh->m_IndexBufferView = ibv;

    renderer.GetCommandList()->IASetVertexBuffers(0, 1, &mesh->GetVertexBufferView());
    renderer.GetCommandList()->IASetIndexBuffer(&mesh->GetIndexBufferView());
    renderer.GetCommandList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

    return true;
}

bool GeometryLoader::LoadTexturesFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel)
{
    size_t textureCount = gltfModel.textures.size();

    if (textureCount == 0)
    {
        Debug::LogMessage("No textures found in file.\n");
        return true;
    }

    for (size_t i = 0; i < textureCount; i++)
    {
        //Get texture from list.
        const tinygltf::Texture& gltfTexture = gltfModel.textures[i];
        //Get actual image from image list using source as index.
        const tinygltf::Image& gltfImage = gltfModel.images[gltfTexture.source];

        Texture* texture = new Texture();

        if (gltfImage.uri.empty() && gltfImage.mimeType.empty())
        {
            Debug::LogWarning("Error loading image %i from file: No uri or mimeType found.\n");
            
            //Destroy texture object.
            if (texture != nullptr)
            {
                delete texture;
                texture = nullptr;
            }

            continue;
        }

        bool imageLoaded = false;

        if (gltfImage.uri.empty() == false)
        {
            Debug::LogMessage("Loading image via URI\n");
            //Load image using image location.

            //if (gltfImage.image.size() != 0)
            //{
            //    //Image has been loaded by tinyGLTF. Create texture from buffer.
            //    imageLoaded = TextureLoader::LoadFromData(renderer, *texture, gltfImage.image.data(), gltfImage.image.size());
            //}
            //else
            {
                //Image needs loading from file manually.
                imageLoaded = TextureLoader::LoadFromFile(renderer, *texture, gltfImage.uri);
            }

        }
        else if (gltfImage.mimeType.empty() == false && gltfImage.bufferView != -1)
        {
            Debug::LogMessage("Loading image via mimeType and BufferView.\n");
            //Load image using buffer and mimeType.

            const tinygltf::BufferView& bufferView = gltfModel.bufferViews[gltfImage.bufferView];
            
            const void* buffer = gltfModel.buffers[bufferView.buffer].data.data();
            const size_t bufferSize = gltfModel.buffers[bufferView.buffer].data.size();

            imageLoaded = TextureLoader::LoadFromData(renderer, *texture, buffer, bufferSize);
        }
        else
        {
            Debug::LogWarning("Error loading image %i from file: No uri or mimeType found.\n");
            continue;
        }

        if (imageLoaded)
        {
            //Add to model texture list.
            model.m_Textures.push_back(texture);
        }
        else
        {
            Debug::LogSevere("Error loading image %i from file.\n");

            //Destroy texture object.
            if (texture != nullptr)
            {
                delete texture;
                texture = nullptr;
            }
        }

    }

    return true;

}

bool GeometryLoader::Load(Renderer& renderer, Model& model, const std::string& path)
{
    std::filesystem::path filepath = path;
 
    if (filepath.empty() || std::filesystem::exists(filepath) == false)
    {
        Debug::LogWarning("Tried to load a file using an invalid filepath (empty or does not exist.).\n");
        return false;
    }    

    std::string gltfError = "";
    std::string gltfWarning = "";

    tinygltf::TinyGLTF gltfLoader;
    tinygltf::Model gltfModel;

    bool loadedGltf = false;

    if (filepath.extension() == ".gltf")
    {
        loadedGltf = gltfLoader.LoadASCIIFromFile(&gltfModel, &gltfError, &gltfWarning, filepath.string());
    }
    else if (filepath.extension() == ".glb")
    {
        loadedGltf = gltfLoader.LoadBinaryFromFile(&gltfModel, &gltfError, &gltfWarning, filepath.string());
    }
    else
    {
        Debug::LogWarning("Tried to load a file using an invalid filepath (neither .gltf or .glb).\n");
        return false;
    }

    if (gltfWarning.empty() == false)
    {
        Debug::LogWarning("Warning from GLTF loader: %s\n", gltfWarning.c_str());
    }

    if (gltfError.empty() == false)
    {
        Debug::LogSevere("Error from GLTF loader: %s\n", gltfError.c_str());
        return false;
    }

    if (loadedGltf == false)
    {
        Debug::LogWarning("Failed to parse gltf file: %s.\n", filepath.c_str());
        return false;
    }

    loadedGltf = CreateModelFromGLTF(renderer, model, gltfModel);

    if (loadedGltf == false)
    {
        Debug::LogSevere("Failed to create model from loaded gltf file: %s\n", filepath.c_str());
        Destroy(model);
        return false;
    }

    return loadedGltf;
}

void GeometryLoader::Destroy(Model& model)
{

}
