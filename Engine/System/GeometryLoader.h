#pragma once

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

class GeometryLoader
{
private:
	static bool CreateModelFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel);
	static bool LoadGeometryFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel);
	static bool GetVertexDataFromGLTFPrimitive(std::vector<Vertex>& vertices, const tinygltf::Model& model, const tinygltf::Primitive& primitive);
	static bool GetIndexDataFromGLTFPrimitive(std::vector<uint16_t>& indices, const tinygltf::Model& model, const tinygltf::Primitive& gltfMesh);
	static Mesh* CreateMeshFromData(Renderer& renderer, Model& model, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
	static bool LoadTexturesFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel);

public:
	static bool Load(Renderer& renderer, Model& model, const std::string& path);
	static void Destroy(Model& model);
};

