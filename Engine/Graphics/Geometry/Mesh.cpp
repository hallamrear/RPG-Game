#include "pch.h"
#include "Mesh.h"
#include <Graphics/Renderer.h>

Mesh::Mesh()
{
    m_Name = "Unnamed Mesh";
    m_TopologyType = D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    m_VertexBuffer = nullptr;
    m_VertexBufferView = D3D12_VERTEX_BUFFER_VIEW{};
    m_VertexCount = -1;
    m_IndexBuffer = nullptr;
    m_IndexBufferView = D3D12_INDEX_BUFFER_VIEW{};
    m_IndexCount = -1;
    m_UsesIndexBuffer = false;
    m_MaxPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_MinPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

Mesh::~Mesh()
{
    m_Name = "DELETED MESH";

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
    m_VertexCount = -1;

    m_IndexBufferView = D3D12_INDEX_BUFFER_VIEW{};
    m_IndexCount = -1;
    m_UsesIndexBuffer = false;

    m_MaxPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_MinPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

const std::string& Mesh::GetName() const
{
    return m_Name;
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

const size_t Mesh::GetVertexCount() const
{
    return m_VertexCount;
}

const size_t Mesh::GetIndexCount() const
{
    return m_IndexCount;
}

const DirectX::XMFLOAT3& Mesh::GetMaxPosition() const
{
    return m_MaxPosition;
}

const DirectX::XMFLOAT3& Mesh::GetMinPosition() const
{
    return m_MinPosition;
}

void Mesh::Render(Renderer& renderer) const
{
    renderer.GetCommandList()->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    renderer.GetCommandList()->IASetPrimitiveTopology(m_TopologyType);

    if (m_UsesIndexBuffer)
    {
        renderer.GetCommandList()->IASetIndexBuffer(&m_IndexBufferView);
        renderer.GetCommandList()->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
    }
    else
    {
        renderer.GetCommandList()->DrawInstanced(m_VertexCount, 1, 0, 0);
    }
}
