#pragma once

namespace tinygltf
{
	class Model;
	struct Primitive;
	struct Mesh;
}

class Renderer;
class World;

class SceneLoader
{
private:
	static bool LoadSceneFromGLTF(Renderer& renderer, World& world, tinygltf::Model& model);
	static void Destroy(World& world);

public:
	static bool LoadSceneFromFileIntoWorld(Renderer& renderer, World& world, const std::string& path);
};

