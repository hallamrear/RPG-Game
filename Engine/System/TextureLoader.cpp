#include "pch.h"
#include "TextureLoader.h"
#include <System/FileLoadingIncludes.h>
#include <Graphics/Texturing/Texture.h>

bool TextureLoader::LoadFromFile(Texture& texture, const std::string& path)
{
	return false;
}

void TextureLoader::Destroy(Texture& texture)
{
	if (texture.IsLoaded())
	{
		texture.Destroy();
	}
}