#pragma once

class Texture;
class Renderer;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;

class TextureLoader
{
private:
	static D3D12_SHADER_RESOURCE_VIEW_DESC GetTexture2DResourceViewDescription();
	
public:
	static bool LoadFromData(Renderer& renderer, Texture& texture, const void* data, const size_t& bytes, const int& width, const int& height);
	static bool LoadFromFile(Renderer& renderer, Texture& texture, const std::string& path);
	static void Destroy(Texture& texture);
};

