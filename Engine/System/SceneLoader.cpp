#include "pch.h"
#include "SceneLoader.h"
#include <World/World.h>
#include <World/Entity.h>
#include <System/Debug.h>


#ifndef TINYGLTF_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#include <System/FileLoadingIncludes.h>

bool SceneLoader::LoadSceneFromFileIntoWorld(Renderer& renderer, World& world, const std::string& path)
{
    std::filesystem::path filepath = path;

    if (filepath.empty() || std::filesystem::exists(filepath) == false)
    {
        Debug::LogWarning("Tried to load a file using an invalid filepath (empty or does not exist.).\n");
        return false;
    }

    Debug::LogMessage("Loading file: %s\n", path.c_str());

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

    loadedGltf = LoadSceneFromGLTF(renderer, world, gltfModel);

    if (loadedGltf == false)
    {
        Debug::LogSevere("Failed to create model from loaded gltf file: %s\n", filepath.c_str());
        Destroy(world);
        return false;
    }

    return loadedGltf;
}

void SceneLoader::Destroy(World& world)
{
    
}

bool SceneLoader::LoadSceneFromGLTF(Renderer& renderer, World& world, tinygltf::Model& model)
{
    size_t sceneCount = model.scenes.size();
    Debug::LogMessage("Found %i scenes.\n", sceneCount);

    if (sceneCount <= 0)
    {
        Debug::LogSevere("File has no scenes.\n");
        return false;
    }

    size_t nodeCount = model.nodes.size();
    Debug::LogMessage("Found %i nodes.\n", nodeCount);

    if (nodeCount <= 0)
    {
        Debug::LogSevere("File has no nodes.\n");
        return false;
    }

    std::vector<int> nodesWithChildren = std::vector<int>();

    size_t preloadEntityIndex = world.m_Entities.size();
    for (size_t i = 0; i < nodeCount; i++)
    {
        world.m_Entities.push_back(new Entity());
        Entity& entity = *world.m_Entities.back();
        entity.SetName(model.nodes[i].name);

        if (model.nodes[i].children.size() > 0)
        {
            nodesWithChildren.push_back(i);
        }

        DirectX::XMFLOAT4X4 localMatrix;
        DirectX::XMStoreFloat4x4(&localMatrix, DirectX::XMMatrixIdentity());

        if (model.nodes[i].matrix.size() == 16)
        {
        }
        else
        {
            DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
            DirectX::XMFLOAT3 translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

            if (model.nodes[i].scale.size() == 3)
            {
                scale.x = model.nodes[i].scale[0];
                scale.y = model.nodes[i].scale[1];
                scale.z = model.nodes[i].scale[2];
            }

            if (model.nodes[i].rotation.size() == 4)
            {
                rotation.x = model.nodes[i].rotation[0];
                rotation.y = model.nodes[i].rotation[1];
                rotation.z = model.nodes[i].rotation[2];
                rotation.w = model.nodes[i].rotation[3];
            }

            if (model.nodes[i].translation.size() == 3)
            {
                translation.x = model.nodes[i].translation[0];
                translation.y = model.nodes[i].translation[1];
                translation.z = model.nodes[i].translation[2];
            }

            DirectX::XMStoreFloat4x4(&localMatrix, 
                DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
                DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation)) *
                DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z));
        }

        entity.SetLocalMatrix(localMatrix);
    }

    int parentNodeIndex = -1;
    int parentWorldIndex = -1;
    int childWorldIndex = -1;
    Entity* parent = nullptr;
    Entity* child = nullptr;
    for (size_t c = 0; c < nodesWithChildren.size(); c++)
    {
        int parentNodeIndex = nodesWithChildren[c];
        size_t childCount = model.nodes[parentNodeIndex].children.size();

        for (size_t i = 0; i < childCount; i++)
        {
            parentWorldIndex = preloadEntityIndex + parentNodeIndex;
            childWorldIndex = preloadEntityIndex + model.nodes[parentNodeIndex].children[i];
            parent = world.m_Entities[parentWorldIndex];
            child = world.m_Entities[childWorldIndex];

            if (child != nullptr && parent != nullptr)
            {
                child->SetParent(parent);
            }
            
            parent = nullptr;
            child = nullptr;
            parentWorldIndex = -1;
            childWorldIndex = -1;
        }

        parentNodeIndex = -1;
    }

    return true;
}