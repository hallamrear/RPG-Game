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

    return false;
}

#include <Graphics/ColourOnlyVertex.h>

bool GeometryLoader::LoadGeometryFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel)
{

    ColourOnlyVertex vertices[] =
    {
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }
    };

    size_t vbSize = sizeof(ColourOnlyVertex) * 8;

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

    //pg 257
    //createblob(ibBlob)
    //copymem(ibblob->indexbufferCPU)

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
