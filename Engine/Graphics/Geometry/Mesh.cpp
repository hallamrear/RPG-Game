#include "pch.h"
#include "Mesh.h"

Mesh::Mesh()
{
    m_VertexBuffer = nullptr;
    m_VertexBufferView = D3D12_VERTEX_BUFFER_VIEW{};

    m_IndexBuffer = nullptr;
    m_IndexBufferView = D3D12_INDEX_BUFFER_VIEW{};
}

Mesh::~Mesh()
{
    if (m_VertexBuffer != nullptr)
    {
        m_VertexBuffer->Release();
        m_VertexBuffer = nullptr;
    }

    if (m_IndexBuffer != nullptr)
    {
        m_IndexBuffer->Release();
        m_IndexBuffer = nullptr;
    }

    m_VertexBufferView = D3D12_VERTEX_BUFFER_VIEW{};
    m_IndexBufferView = D3D12_INDEX_BUFFER_VIEW{};
}

const ID3D12Resource* Mesh::GetVertexBuffer() const
{
    return m_VertexBuffer;
}

const D3D12_VERTEX_BUFFER_VIEW& Mesh::GetVertexBufferView() const
{
    return m_VertexBufferView;
}

const ID3D12Resource* Mesh::GetIndexBuffer() const
{
    return m_IndexBuffer;
}

const D3D12_INDEX_BUFFER_VIEW& Mesh::GetIndexBufferView() const
{
    return m_IndexBufferView;
}
