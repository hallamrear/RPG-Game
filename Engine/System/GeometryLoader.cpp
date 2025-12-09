#include "pch.h"
#include "GeometryLoader.h"
#include <System/Debug.h>
#include <System/FileLoadingIncludes.h>
#include <System/TextureLoader.h>
#include <Graphics/Texturing/Texture.h>
#include <Graphics/Vertex.h>
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

bool GeometryLoader::LoadGeometryFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel)
{
    size_t meshCount = gltfModel.meshes.size();
    size_t primitiveCount = 0;
    size_t primitiveTargetCount = 0;
    size_t primitiveAttributeCount = 0;

    D3D12_PRIMITIVE_TOPOLOGY foundTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    std::vector<Vertex> vertices = std::vector<Vertex>();
    std::vector<uint16_t> indices = std::vector<uint16_t>();
    std::vector<uint16_t> weights = std::vector<uint16_t>();
    std::vector<uint16_t> joints = std::vector<uint16_t>();
    DirectX::XMFLOAT3 max = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    DirectX::XMFLOAT3 min = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);


    for (size_t i = 0; i < meshCount; i++)
    {
        tinygltf::Mesh& gltfMesh = gltfModel.meshes[i];
        
        primitiveCount = gltfModel.meshes[i].primitives.size();
        primitiveTargetCount = 0;

        vertices.clear();
        indices.clear();
        weights.clear();
        joints.clear();

        for (size_t p = 0; p < primitiveCount; p++)
        {
            tinygltf::Primitive& primitive = gltfMesh.primitives[p];
            
            switch (primitive.mode)
            {
            case TINYGLTF_MODE_POINTS: { foundTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST; } break;
            case TINYGLTF_MODE_LINE: { foundTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST; } break;
            case TINYGLTF_MODE_LINE_STRIP: { foundTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINESTRIP; } break;
            case TINYGLTF_MODE_TRIANGLES: { foundTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; } break;
            case TINYGLTF_MODE_TRIANGLE_STRIP: { foundTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; } break;
            case TINYGLTF_MODE_TRIANGLE_FAN: { foundTopology = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN; } break;
            
            case TINYGLTF_MODE_LINE_LOOP:
            default:
                Debug::LogSevere("Unsupported primitive topology in gltf file.\n");
                return false;
                break;
            }

            int indicesBufferIndex = primitive.indices;

            bool usesIndexBuffer = (indicesBufferIndex > 0);

            primitiveAttributeCount = primitive.attributes.size();
            
            if (primitiveAttributeCount <= 0)
            {
                Debug::LogSevere("No primitive attributes found.\n");
                return false;
            }

            bool loadedVertices = GetVertexDataFromGLTFPrimitive(vertices, gltfModel, primitive, max, min);

            if (loadedVertices == false)
            {
                Debug::LogSevere("Failed to load vertices from gltf model.\n");
                return false;
            }

            bool loadedIndices = false;
            if (usesIndexBuffer)
            {
                loadedIndices = GetIndexDataFromGLTFPrimitive(indices, gltfModel, primitive);
            }

            if (loadedIndices == false)
            {
                Debug::LogWarning("Loading a gltf model that does not seem to use indices.\n");
            }
        }

        Mesh* mesh = CreateMeshFromData(renderer, model, vertices, indices);

        if (mesh == nullptr)
        {
            Debug::LogSevere("Failed to load mesh data from gltf.\n");
            return false;
        }

        mesh->m_TopologyType = foundTopology;
        mesh->m_MaxPosition = max;
        mesh->m_MinPosition = min;

        primitiveCount = 0;
        max = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        min = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    return true;
}

bool GeometryLoader::GetMaterialFromGLTFPrimitive(Material& material, const tinygltf::Model& model, const tinygltf::Primitive& primitive)
{
    //TODO : Material renderering.
    int materialIndex = primitive.material;
    tinygltf::Material gltfMaterial = model.materials[materialIndex];
    return true;
}

bool GeometryLoader::GetElementDataFromGLTFBuffer(const std::string& attributeName, byte*& data, tinygltf::Model& model, tinygltf::Primitive& primitive)
{
    bool hasAttribute = (primitive.attributes.find(attributeName) != primitive.attributes.end());

    if (hasAttribute)
    {
        const int& attributeAccessorIndex = primitive.attributes.at(attributeName);
        tinygltf::Accessor& attributeAccessor = model.accessors[attributeAccessorIndex];
        tinygltf::BufferView& attributeBufferView = model.bufferViews[attributeAccessor.bufferView];
        tinygltf::Buffer& attributeBuffer = model.buffers[attributeBufferView.buffer];
        size_t elementSize = tinygltf::GetComponentSizeInBytes(attributeAccessor.componentType) * tinygltf::GetNumComponentsInType(attributeAccessor.type);
        data = new byte[elementSize * attributeAccessor.count];
        memset(data, 0, elementSize * attributeAccessor.count * sizeof(byte));
        void* src = (attributeBuffer.data.data() + attributeBufferView.byteOffset + attributeAccessor.byteOffset);
        memcpy_s(data, elementSize * attributeAccessor.count, src, attributeBufferView.byteLength);
    }

    return hasAttribute;
}

bool GeometryLoader::GetVertexDataFromGLTFPrimitive(std::vector<Vertex>& vertices, tinygltf::Model& model, tinygltf::Primitive& primitive, DirectX::XMFLOAT3& maxPosition, DirectX::XMFLOAT3& minPosition)
{
    size_t primitiveAttributeCount = primitive.attributes.size();

    if (primitiveAttributeCount <= 0)
    {
        Debug::LogSevere("No primitive attributes found for vertices.\n");
        return false;
    }
    
    //The count of the vertices is defined by the spec.
    //All attribute accessors for each primitive MUST have the same count.
    //So directly accessing the position attribute count should provide enough to fill the vector.
  
    byte* positionData = nullptr;
    byte* normalData = nullptr;
    byte* tangentData = nullptr;
    byte* texCoordData = nullptr;
    byte* jointData = nullptr;
    byte* weightData = nullptr;

    bool hasPositions = GetElementDataFromGLTFBuffer("POSITION", positionData, model, primitive);
    bool hasNormals = GetElementDataFromGLTFBuffer("NORMAL", normalData, model, primitive);
    bool hasTangents = GetElementDataFromGLTFBuffer("TANGENT", tangentData, model, primitive);
    bool hasTexCoords = GetElementDataFromGLTFBuffer("TEXCOORD_0", texCoordData, model, primitive);
    bool hasJoints = GetElementDataFromGLTFBuffer("JOINTS_0", jointData, model, primitive);
    bool hasWeights = GetElementDataFromGLTFBuffer("WEIGHTS_0", weightData, model, primitive);

    if (hasPositions == false)
    {
        Debug::LogSevere("Model does not contain any position data.\n");
        return false;
    }

    size_t originalVertexIndex = vertices.size();
    size_t expectedVertexCount = model.accessors[primitive.attributes.at("POSITION")].count;
    vertices.resize(originalVertexIndex + expectedVertexCount);

    maxPosition.x = model.accessors[primitive.attributes.at("POSITION")].maxValues[0];
    maxPosition.y = model.accessors[primitive.attributes.at("POSITION")].maxValues[1];
    maxPosition.z = model.accessors[primitive.attributes.at("POSITION")].maxValues[2];

    minPosition.x = model.accessors[primitive.attributes.at("POSITION")].minValues[0];
    minPosition.y = model.accessors[primitive.attributes.at("POSITION")].minValues[1];
    minPosition.z = model.accessors[primitive.attributes.at("POSITION")].minValues[2];

    int index = 0;
    for (size_t i = 0; i < expectedVertexCount; i++)
    {
        index = originalVertexIndex + i;

        if (hasPositions && positionData != nullptr)
        {
            vertices[index].Position = *(DirectX::XMFLOAT3*)(positionData + (sizeof(DirectX::XMFLOAT3) * i));
        }

        if (hasNormals && normalData != nullptr)
        {
            vertices[index].Normal = *(DirectX::XMFLOAT3*)(normalData + (sizeof(DirectX::XMFLOAT3) * i));
        }

        if (hasTangents && tangentData != nullptr)
        {
            vertices[index].Tangent = *(DirectX::XMFLOAT3*)(tangentData + (sizeof(DirectX::XMFLOAT3) * i));
        }

        if (hasTexCoords && texCoordData != nullptr)
        {
            vertices[index].UV = *(DirectX::XMFLOAT2*)(texCoordData + (sizeof(DirectX::XMFLOAT2) * i));
        }

        //if (hasJoints && jointData != nullptr)
        //{
        //    vertices[index].Joints = *jointData[i];
        //}
        //
        //if (hasJoints && weightData != nullptr)
        //{
        //    vertices[index].Weights = *weightData[i];
        //}
    }

    if (positionData != nullptr)
    {
        delete[] positionData;
        positionData = nullptr;
    }

    if (normalData != nullptr)
    {
        delete[] normalData;
        normalData = nullptr;
    }

    if (tangentData != nullptr)
    {
        delete[] tangentData;
        tangentData = nullptr;
    }

    if (texCoordData != nullptr)
    {
        delete[] texCoordData;
        texCoordData = nullptr;
    }

    if (jointData != nullptr)
    {
        delete[] jointData;
        jointData = nullptr;
    }

    if (weightData != nullptr)
    {
        delete[] weightData;
        weightData = nullptr;
    }

    return true;
}

bool GeometryLoader::GetIndexDataFromGLTFPrimitive(std::vector<uint16_t>& indices, const tinygltf::Model& model, const tinygltf::Primitive& primitive)
{
    int indicesAccessorIndex = primitive.indices;

    if (indicesAccessorIndex <= 0)
    {
        Debug::LogWarning("Failed to find a valid indices accessor index.\n");
        return false;
    }
        
    tinygltf::Accessor indicesAccessor = model.accessors[indicesAccessorIndex];

    switch (indicesAccessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
    {

    }
    break;

    case TINYGLTF_TYPE_VEC2:
    case TINYGLTF_TYPE_VEC3:
    case TINYGLTF_TYPE_VEC4:
    case TINYGLTF_TYPE_MAT2:
    case TINYGLTF_TYPE_MAT3:
    case TINYGLTF_TYPE_MAT4:
    case TINYGLTF_TYPE_VECTOR:
    case TINYGLTF_TYPE_MATRIX:
    default:
        Debug::LogSevere("Unsupported type for index buffer.\n");
        return false;
        break;
    }

    switch (indicesAccessor.componentType)
    {

    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
    {

    }
    break;

    case TINYGLTF_PARAMETER_TYPE_BYTE:
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
    case TINYGLTF_PARAMETER_TYPE_SHORT:
    case TINYGLTF_PARAMETER_TYPE_INT:
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
    case TINYGLTF_PARAMETER_TYPE_FLOAT:
    default:
        Debug::LogSevere("Currently unsupported component type for index buffer.\n");
        return false;
        break;
    }

    size_t expectedIndexCount = indicesAccessor.count;

    int indicesBufferViewIndex = indicesAccessor.bufferView;
    int indicesAccessorByteOffset = indicesAccessor.byteOffset;

    const tinygltf::BufferView& indicesBufferView = model.bufferViews[indicesBufferViewIndex];
    int indicesByteStride = indicesAccessor.ByteStride(indicesBufferView);

    if (indicesByteStride == -1)
    {
        Debug::LogSevere("Failed to get buffer view stride.\n");
        return false;
    }

    if (indicesByteStride != sizeof(uint16_t))
    {
        Debug::LogSevere("ByteStride from buffer view and expected index size do not match.\n");
        return false;
    }

    size_t elementSize = tinygltf::GetComponentSizeInBytes(indicesAccessor.componentType) * tinygltf::GetNumComponentsInType(indicesAccessor.type);

    int indicesBufferIndex = indicesBufferView.buffer;
    const tinygltf::Buffer& indicesBuffer = model.buffers[indicesBufferIndex];
    size_t indicesBufferViewByteOffset = indicesBufferView.byteOffset;

    size_t originalIndicesIndex = indices.size();
    indices.resize(indices.size() + expectedIndexCount);

    size_t dataSize = elementSize * expectedIndexCount;
    void* dst = &indices[originalIndicesIndex];
    const void* src = indicesBuffer.data.data() + indicesBufferViewByteOffset + indicesAccessorByteOffset;

    memcpy(dst, src, dataSize);

    return true;
}

Mesh* GeometryLoader::CreateMeshFromData(Renderer& renderer, Model& model, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
{
    ID3D12Resource* vertexBuffer = nullptr;
    ID3D12Resource* vbUploader = nullptr;
    ID3D12Resource* indexBuffer = nullptr;
    ID3D12Resource* ibUploader = nullptr;

    size_t vbSize = sizeof(Vertex) * vertices.size();

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
    vbv.StrideInBytes = sizeof(Vertex);

    size_t ibSize = sizeof(uint16_t) * indices.size();

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

    return mesh;
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
