#pragma once

namespace tinygltf
{
	class Model;
	struct Primitive;
	struct Mesh;
	class Node;
}

class Renderer;
class World;

class SceneLoader
{
private:
	static bool LoadSceneFromGLTF(Renderer& renderer, World& world, tinygltf::Model& model, const std::string& parentPath);
	static void BuildHierarchy(const int& preloadEntityIndex, World& world, const std::vector<tinygltf::Node>& gltfNodes, std::vector<int>& nodesWithChildren);
	static void Destroy(World& world);

public:
	static bool LoadSceneFromFileIntoWorld(Renderer& renderer, World& world, const std::string& path);
};

