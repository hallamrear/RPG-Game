#pragma once
#include <Graphics/DX12Includes.h>

struct ID3D12Resource;

class Texture
{
private:
	friend class TextureLoader;
	int m_ID;
	int m_Width;
	int m_Height;
	bool m_IsLoaded;
	ID3D12Resource* m_Resource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_SRVHandle;

	void Destroy();

public:
	Texture();
	~Texture();

	ID3D12Resource* GetResource() const;
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetSRVHandle() const;
	const int& GetID() const;
	const bool& IsLoaded() const;
};

