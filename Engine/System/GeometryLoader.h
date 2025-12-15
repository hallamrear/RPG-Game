#pragma once
#include <DirectXMath.h>

struct Vertex;

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
	static bool GetMaterialFromGLTFPrimitive(Material& vertices, const tinygltf::Model& model, const tinygltf::Primitive& primitive);
	static bool GetVertexDataFromGLTFPrimitive(std::vector<Vertex>& vertices, tinygltf::Model& model, tinygltf::Primitive& primitive, DirectX::XMFLOAT3& maxPosition, DirectX::XMFLOAT3& minPosition);
	static bool GetIndexDataFromGLTFPrimitive(std::vector<uint16_t>& indices, const tinygltf::Model& model, const tinygltf::Primitive& gltfMesh);
	static Mesh* CreateMeshFromData(Renderer& renderer, Model& model, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
	static bool LoadTexturesFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel, const std::string& parentPath);
	static bool GetElementDataFromGLTFBuffer(const std::string& attributeName, byte*& data, tinygltf::Model& model, tinygltf::Primitive& primitive);

public:
	static bool Load(Renderer& renderer, Model& model, const std::string& path);
	static void Destroy(Model& model);
};

