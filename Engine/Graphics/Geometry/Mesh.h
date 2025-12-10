#pragma once
#include <Graphics/DX12Includes.h>

class Renderer;

class Mesh
{
private:
	friend class GeometryLoader;
	friend class Model;

	std::string m_Name;

	D3D12_PRIMITIVE_TOPOLOGY m_TopologyType;

	DirectX::XMFLOAT3 m_MaxPosition;
	DirectX::XMFLOAT3 m_MinPosition;

	ID3D12Resource* m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	ID3D12Resource* m_IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	bool m_UsesIndexBuffer;
	size_t m_VertexCount;
	size_t m_IndexCount;

protected:

public:
	Mesh();
	~Mesh();

	const std::string& GetName() const;

	const ID3D12Resource* GetVertexBuffer() const;
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const;

	const ID3D12Resource* GetIndexBuffer() const;
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const;

	const size_t GetVertexCount() const;
	const size_t GetIndexCount() const;

	const DirectX::XMFLOAT3& GetMaxPosition() const;
	const DirectX::XMFLOAT3& GetMinPosition() const;

	void Render(Renderer& renderer) const;
};

