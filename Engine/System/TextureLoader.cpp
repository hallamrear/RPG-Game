#include "pch.h"
#include "TextureLoader.h"
#include <Graphics/Texturing/Texture.h>
#include <External/stb_image.h>
#include <fstream>
#include <iostream>
#include <Graphics/DX12Includes.h>
#include <Graphics/Renderer.h>
#include <System/Debug.h>

std::unordered_map<std::string, Texture*> TextureLoader::m_TextureMap = std::unordered_map<std::string, Texture*>();

bool TextureLoader::IsTextureLoaded(const std::string& filename)
{
	auto itr = m_TextureMap.find(filename);
	return itr != m_TextureMap.end();
}

bool TextureLoader::CreateNullDescriptor(Renderer& renderer, Texture& texture)
{
	return false;
}

bool TextureLoader::LoadFromData(Renderer& renderer, Texture& texture, const std::string& referenceName, const void* data, const size_t& bytes)
{
	if (texture.IsLoaded())
	{
		Debug::LogSevere("Creating an object in an already initialised texture.\n");
		return false;
	}

	ID3D12Device* device = renderer.GetDevice();
	ID3D12GraphicsCommandList* commandList = renderer.GetCommandList();

	if (device == nullptr || commandList == nullptr)
	{
		Debug::LogSevere("Returned an invalid renderer object during texture creation.\n");
		return false;
	}

	const DXGI_FORMAT textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	int width = -1;
	int height = -1;
	int channels = -1;

	unsigned char* pixels = stbi_load_from_memory((const stbi_uc*)data, bytes, &width, &height, &channels, 4);

	if (pixels == nullptr || width == -1 || height == -1 || channels == -1)
	{
		Debug::LogSevere("Failed to load texture from data.\n");
		return false;
	}

	size_t bytesPerPixel = sizeof(unsigned char) * 4 * channels;

	D3D12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(textureFormat, width, height);
	HRESULT result = device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture.m_Resource));

	if (FAILED(result))
	{
		Debug::LogSevere("Failed to create commited resource for texture.\n");
		return SUCCEEDED(result);
	}

	ID3D12Resource* textureUploadHeap = nullptr;
	D3D12_HEAP_PROPERTIES cpuUploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);
	UINT64 uploadSize = GetRequiredIntermediateSize(texture.m_Resource, 0, 1);

	CD3DX12_RESOURCE_DESC gpuUploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

	result = device->CreateCommittedResource(
		&cpuUploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&gpuUploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap));

	if (FAILED(result))
	{
		Debug::LogSevere("Failed to create commited BPU resources (upload heap) for texture.\n");
		return SUCCEEDED(result);
	}

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = pixels;
	textureData.RowPitch = (width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
	textureData.SlicePitch = textureData.RowPitch * height;

	UpdateSubresources(commandList, texture.m_Resource, textureUploadHeap, 0, 0, 1, &textureData);

	CD3DX12_RESOURCE_BARRIER copyToSRVTransition = CD3DX12_RESOURCE_BARRIER::Transition(texture.m_Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &copyToSRVTransition);
	 
	CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2D(textureFormat);

	//Map Size + 1 to account for the null descriptor created by the renderer.
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(renderer.GetMainSRVDescriptorHeapStartCPU(), m_TextureMap.size() + 1, renderer.GetSRVDescriptorHeapSize());

	device->CreateShaderResourceView(texture.m_Resource, &srvDesc, srvCpuHandle);

	result = renderer.ExecuteAndResetCommandList();

	if (FAILED(result))
	{
		Debug::LogSevere("Failed to execute and reset command list during texture buffer creation.\n");
		return SUCCEEDED(result);
	}

	if (texture.m_Resource != nullptr)
	{
		texture.m_Height = height;
		texture.m_Width = width;
		texture.m_IsLoaded = true;
		texture.m_ID = m_TextureMap.size();
		texture.m_CPUHandle = srvCpuHandle;
		m_TextureMap.insert({ referenceName, &texture });		
	}

	return texture.m_IsLoaded;
}

bool TextureLoader::LoadExistingResourceFromMap(Texture& texture, const std::string& path)
{
	std::unordered_map<std::string, Texture*>::iterator itr = m_TextureMap.find(path);

	if (itr == m_TextureMap.end())
	{
		Debug::LogSevere("Texture map thinks it exists but searching does not. This is bad.");
		return false;
	}

	texture = *itr->second;

	return true;
}

bool TextureLoader::LoadFromFile(Renderer& renderer, Texture& texture, const std::string& path)
{
	if (texture.IsLoaded())
	{
		Debug::LogSevere("Creating an object in an already initialised texture.\n");
		return false;
	}

	if (IsTextureLoaded(path))
	{
		Debug::LogWarning("Texture already exists in map. Populating texture with their values.\n");
		return LoadExistingResourceFromMap(texture, path);
	}

	std::fstream imageFile(path, std::ios::in | std::ios::binary | std::ios::ate);

	if (!imageFile.good())
	{
		Debug::LogSevere("Failed to load texture from file %s.\n", path.c_str());
		return false;
	}

	if (!imageFile.is_open())
	{
		Debug::LogSevere("Failed to load texture from file %s.\n", path.c_str());
		return false;
	}

	int bufferSize = (int)imageFile.tellg();
	imageFile.seekg(0, std::ios::beg);

	char* buffer = new char[bufferSize];
	imageFile.read(buffer, bufferSize);
	imageFile.close();
	
	if (buffer == nullptr)
	{
		Debug::LogSevere("Failed to load texture from file %s.\n", path.c_str());
		delete[] buffer;
		buffer = nullptr;
		return false;
	}

	bool imageLoaded = LoadFromData(renderer, texture, path, buffer, bufferSize);

	if (imageLoaded == false)
	{
		Debug::LogSevere("Failed to load texture data from file %s.\n", path);
	}

	delete[] buffer;
	buffer = nullptr;
	
	return imageLoaded;
}

void TextureLoader::Destroy(Texture& texture)
{
	if (texture.IsLoaded())
	{
		texture.Destroy();
	}
}