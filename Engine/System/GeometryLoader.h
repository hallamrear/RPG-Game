#pragma once

namespace tinygltf
{
	class Model;
}

class Model;
class Renderer;

class GeometryLoader
{
private:
	static bool CreateModelFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel);
	static bool LoadGeometryFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel);
	static bool LoadTexturesFromGLTF(Renderer& renderer, Model& model, tinygltf::Model& gltfModel);

public:
	static bool Load(Renderer& renderer, Model& model, const std::string& path);
	static void Destroy(Model& model);
};

