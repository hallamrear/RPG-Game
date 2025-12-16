#pragma once
#include <unordered_map>

class Texture;
class Renderer;
struct ID3D12Resource;

class TextureLoader
{
private:
	static std::unordered_map<std::string, Texture*> m_TextureMap;
	static bool IsTextureLoaded(const std::string& filename);
	static bool LoadExistingResourceFromMap(Texture& texture, const std::string& path);
	
public:
	static bool CreateNullDescriptor(Renderer& renderer, Texture& texture);
	static bool LoadFromData(Renderer& renderer, Texture& texture, const std::string& referenceName, const void* data, const size_t& bytes);
	static bool LoadFromFile(Renderer& renderer, Texture& texture, const std::string& path);
	static void Destroy(Texture& texture);
};

