#pragma once

class Texture;

class TextureLoader
{
private:

public:
	static bool LoadFromFile(Texture& texture, const std::string& path);
	static bool LoadFromData(Texture& texture, const std::string& path);
	static void Destroy(Texture& texture);
};

