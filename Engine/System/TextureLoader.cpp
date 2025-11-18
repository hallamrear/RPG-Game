#include "pch.h"
#include "TextureLoader.h"
#include <Graphics/Texturing/Texture.h>
#include <Graphics/DX12Includes.h>
#include <Graphics/Renderer.h>
#include <System/Debug.h>

D3D12_SHADER_RESOURCE_VIEW_DESC  TextureLoader::GetTexture2DResourceViewDescription()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	
	return srvDesc;
}

bool TextureLoader::LoadFromData(Renderer& renderer, Texture& texture, const void* data, const size_t& bytes, const int& width, const int& height)
{
	if (texture.IsLoaded())
	{
		Debug::LogSevere("Creating an object in an already initialised texture.\n");
		return false;
	}

	ID3D12Device* device = renderer.GetDevice();

	if (device == nullptr)
	{
		Debug::LogSevere("Returned an invalid device object during texture creation.\n");
		return false;
	}

	const uint32_t tempPixel = 0xFFFFFFFF;
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &tempPixel;
	//textureData.RowPitch = textureDesc.Width * 4;
	//textureData.SlicePitch = textureDesc.Width * 4;
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = GetTexture2DResourceViewDescription();

	//device->CreateShaderResourceView(texture.m_Resource, &srvDesc, destDescriptor);

	return true;
}

bool TextureLoader::LoadFromFile(Renderer& renderer, Texture& texture, const std::string& path)
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