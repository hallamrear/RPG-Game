#pragma once
#include <Graphics/DX12Includes.h>

class Mesh
{
private:
	friend class GeometryLoader;
	friend class Model;

	ID3D12Resource* m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	ID3D12Resource* m_IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

protected:

public:
	Mesh();
	~Mesh();

	const ID3D12Resource* GetVertexBuffer() const;
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const;

	const ID3D12Resource* GetIndexBuffer() const;
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const;
};

