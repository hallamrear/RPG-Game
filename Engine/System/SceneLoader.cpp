#include "pch.h"
#include "SceneLoader.h"
#include <World/World.h>
#include <World/Entity.h>
#include <System/Debug.h>
#include <System/FileLoadingIncludes.h>
#include <System/GeometryLoader.h>
#include <Graphics/Geometry/Model.h>

#include <System/TextureLoader.h>
#include <Graphics/Texturing/Texture.h>

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

    loadedGltf = LoadSceneFromGLTF(renderer, world, gltfModel, filepath.parent_path().string());

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

bool SceneLoader::LoadSceneFromGLTF(Renderer& renderer, World& world, tinygltf::Model& gltfModel, const std::string& parentPath)
{
    size_t sceneCount = gltfModel.scenes.size();
    Debug::LogMessage("Found %i scenes.\n", sceneCount);

    if (sceneCount <= 0)
    {
        Debug::LogSevere("File has no scenes.\n");
        return false;
    }

    size_t nodeCount = gltfModel.nodes.size();
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
        entity.SetName(gltfModel.nodes[i].name);

        if (gltfModel.nodes[i].children.size() > 0)
        {
            nodesWithChildren.push_back(i);
        }

        DirectX::XMFLOAT4X4 localMatrix;
        DirectX::XMStoreFloat4x4(&localMatrix, DirectX::XMMatrixIdentity());

        Debug::LogMessage("Node [%i] - %s\n", i, gltfModel.nodes[i].name.c_str());
        Debug::LogMessage("Node transform found as ", i);

        if (gltfModel.nodes[i].matrix.size() == 16)
        {
            Debug::LogMessage("matrix.\n");
            //Should copy 16 floats.
            memcpy(&localMatrix, gltfModel.nodes[i].matrix.data(), sizeof(float) * gltfModel.nodes[i].matrix.size());
        }
        else
        {
            Debug::LogMessage("seperate components.\n");
            DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
            DirectX::XMFLOAT3 translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

            if (gltfModel.nodes[i].scale.size() == 3)
            {
                scale.x = gltfModel.nodes[i].scale[0];
                scale.y = gltfModel.nodes[i].scale[1];
                scale.z = gltfModel.nodes[i].scale[2];
            }

            if (gltfModel.nodes[i].rotation.size() == 4)
            {
                rotation.x = gltfModel.nodes[i].rotation[0];
                rotation.y = gltfModel.nodes[i].rotation[1];
                rotation.z = gltfModel.nodes[i].rotation[2];
                rotation.w = gltfModel.nodes[i].rotation[3];
            }

            if (gltfModel.nodes[i].translation.size() == 3)
            {
                translation.x = gltfModel.nodes[i].translation[0];
                translation.y = gltfModel.nodes[i].translation[1];
                translation.z = gltfModel.nodes[i].translation[2];
            }

            Debug::LogMessage("\tScale : { %f, %f, %f }\n", scale.x, scale.y, scale.z);
            Debug::LogMessage("\tRotation : { %f, %f, %f, %f }\n", rotation.x, rotation.y, rotation.z, rotation.w);
            Debug::LogMessage("\tTranslation : { %f, %f, %f }\n", translation.x, translation.y, translation.z);

            DirectX::XMStoreFloat4x4(&localMatrix, 
                DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
                DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation)) *
                DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z));
        }

        entity.SetLocalMatrix(localMatrix);

        if (gltfModel.nodes[i].mesh != -1)
        {
            Model* model = new Model();

            bool loadedMesh = GeometryLoader::LoadGeometryFromGLTFMesh(renderer, *model, gltfModel, gltfModel.nodes[i].mesh);
            loadedMesh &= GeometryLoader::LoadTexturesFromGLTF(renderer, *model, gltfModel, parentPath);

            if (loadedMesh)
            {
                if (model != nullptr)
                {
                    if (entity.GetName() == "Icosphere")
                    {
                        Texture* textures[3] = { new Texture(), new Texture(), new Texture() };
                        TextureLoader::LoadFromFile(renderer, *textures[0], "Resources/ImageA.png");
                        model->AddTexture(textures[0]);
                        TextureLoader::LoadFromFile(renderer, *textures[1], "Resources/ImageB.png");
                        model->AddTexture(textures[1]);
                        TextureLoader::LoadFromFile(renderer, *textures[2], "Resources/ImageC.png");
                        model->AddTexture(textures[2]);
                    }

                    entity.SetModel(model);
                }
            }
            else
            {
                Debug::LogSevere("Failed to load expected mesh from gltf file.\n");

                if (model)
                {
                    delete model;
                    model = nullptr;
                }
            }
        }
    }

    /*
    int parentNodeIndex = -1;
    int parentWorldIndex = -1;
    int childWorldIndex = -1;
    Entity* parent = nullptr;
    Entity* child = nullptr;
    for (size_t c = 0; c < nodesWithChildren.size(); c++)
    {
        int parentNodeIndex = nodesWithChildren[c];
        size_t childCount = gltfModel.nodes[parentNodeIndex].children.size();

        for (size_t i = 0; i < childCount; i++)
        {
            parentWorldIndex = preloadEntityIndex + parentNodeIndex;
            childWorldIndex = preloadEntityIndex + gltfModel.nodes[parentNodeIndex].children[i];
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
    */

    BuildHierarchy(preloadEntityIndex, world, gltfModel.nodes, nodesWithChildren);

    return true;
}

void SceneLoader::BuildHierarchy(const int& preloadEntityIndex, World& world, const std::vector<tinygltf::Node>& gltfNodes, std::vector<int>& nodesWithChildren)
{
    int parentNodeIndex = -1;
    int parentWorldIndex = -1;
    int childWorldIndex = -1;
    int childNodeIndex = -1;
    Entity* parent = nullptr;
    Entity* child = nullptr;
    for (size_t c = 0; c < nodesWithChildren.size(); c++)
    {
        int parentNodeIndex = nodesWithChildren[c];
        size_t childCount = gltfNodes[parentNodeIndex].children.size();

        for (size_t i = 0; i < childCount; i++)
        {
            parentWorldIndex = preloadEntityIndex + parentNodeIndex;
            childNodeIndex = gltfNodes[parentNodeIndex].children[i];
            childWorldIndex = preloadEntityIndex + childNodeIndex;

            parent = world.m_Entities[parentWorldIndex];
            child = world.m_Entities[childWorldIndex];

            if (child != nullptr && parent != nullptr)
            {
                child->SetParent(parent);
            }

            const tinygltf::Node& childNode = gltfNodes[childNodeIndex];
            if (childNode.children.size() > 0)
            {
                nodesWithChildren.push_back(childNodeIndex);
            }

            parent = nullptr;
            child = nullptr;
            parentWorldIndex = -1;
            childNodeIndex = -1;
            childWorldIndex = -1;
        }

        parentNodeIndex = -1;
    }
}