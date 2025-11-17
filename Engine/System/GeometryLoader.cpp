#include "pch.h"
#include "GeometryLoader.h"
#include <System/Debug.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <External/tiny_gltf.h>

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
    std::vector<tinygltf::Texture> textures = gltfModel.textures;


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
