#pragma once

namespace tinygltf
{
	class Model;
}

class Model;

class GeometryLoader
{
private:
	static bool CreateModelFromGLTF(Model& model, tinygltf::Model& gltfModel);
	static bool LoadTexturesFromGLTF(Model& model, tinygltf::Model& gltfModel);

public:
	static bool Load(Model& model, const std::string& path);
	static void Destroy(Model& model);
};

