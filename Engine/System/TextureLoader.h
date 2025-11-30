#pragma once
#include <unordered_map>

class Texture;
class Renderer;
struct ID3D12Resource;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;

class TextureLoader
{
private:
	static std::unordered_map<std::string, ID3D12Resource*> m_TextureMap;
	static bool IsTextureLoaded(const std::string& filename);
	static bool LoadExistingResourceFromMap(Texture& texture, const std::string& path);
	
public:
	static bool LoadFromData(Renderer& renderer, Texture& texture, const void* data, const size_t& bytes);
	static bool LoadFromFile(Renderer& renderer, Texture& texture, const std::string& path);
	static void Destroy(Texture& texture);
};

