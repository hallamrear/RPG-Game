#include "pch.h"
#include "GeometryLoader.h"
#include <System/Debug.h>

#include <System/FileLoadingIncludes.h>

#include <System/TextureLoader.h>
#include <Graphics/Texturing/Texture.h>
#include <Graphics/Geometry/Mesh.h>
#include <Graphics/Geometry/Model.h>

bool GeometryLoader::CreateModelFromGLTF(Model& model, tinygltf::Model& gltfModel)
{
    bool loadedOk = true;

    loadedOk &= LoadTexturesFromGLTF(model, gltfModel);

    return false;
}

bool GeometryLoader::LoadTexturesFromGLTF(Model& model, tinygltf::Model& gltfModel)
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
            continue;
        }

        bool imageLoaded = false;

        if (gltfImage.uri.empty() == false)
        {
            //Check if texture already exists within texture map.

            //Load image using image location.
            imageLoaded = TextureLoader::LoadFromFile(*texture, gltfImage.uri);
        }
        else if (gltfImage.mimeType.empty() == false)
        {
            //Load image using buffer and mimeType.
        }
        else
        {
            Debug::LogWarning("Error loading image %i from file: No uri or mimeType found.\n");
            continue;
        }

        if (imageLoaded)
        {
            //Add to texture map.
            //Add to model texture list.
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

bool GeometryLoader::Load(Model& model, const std::string& path)
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

    loadedGltf = CreateModelFromGLTF(model, gltfModel);

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
