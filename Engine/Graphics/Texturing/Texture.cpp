#include "pch.h"
#include "Texture.h"

void Texture::Destroy()
{
	if (m_IsLoaded && m_Resource != nullptr)
	{
		m_Resource->Release();
		m_Resource = nullptr;
		m_IsLoaded = false;
		m_Height = -1;
		m_Width = -1;
		m_ID = -1;
		m_SRVHandle = {};
	}
}

Texture::Texture()
{
	m_ID = -1;
	m_Resource = nullptr;
	m_Width = -1;
	m_Height = -1;
	m_IsLoaded = false;
	m_SRVHandle = {};
}

Texture::~Texture()
{
	if (m_IsLoaded)
	{
		Destroy();
	}
}

ID3D12Resource* Texture::GetResource() const
{
	if (IsLoaded())
	{
		return m_Resource;
	}

	return nullptr;
}

const D3D12_GPU_DESCRIPTOR_HANDLE& Texture::GetSRVHandle() const
{
	return m_SRVHandle;
}

const int& Texture::GetID() const
{
	return m_ID;
}

const bool& Texture::IsLoaded() const
{
	return m_IsLoaded;
}
