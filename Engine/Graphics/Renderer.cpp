#include "pch.h"
#include "Renderer.h"
#include "Texturing/Texture.h"
#include <filesystem>
#include <fstream>
#include <Graphics/UIImageVertex.h>
#include <Graphics/ColourOnlyVertex.h>
#include <Graphics/BufferStructures.h>
#include <Graphics/Vertex.h>
#include <iostream>
#include <System/Debug.h>

#define FAILED_RETURN(hr) if(FAILED(hr)) return !FAILED(hr);

const int Renderer::m_SwapChainBufferCount = 2;
const DXGI_FORMAT Renderer::m_BackbufferFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
const DXGI_FORMAT Renderer::m_DepthStencilBufferFormat = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;

Renderer::Renderer()
{
    m_OrthoPipeline = nullptr;
    m_DefaultOrthoPixelShaderBlob = nullptr;
    m_DefaultOrthoVertexShaderBlob = nullptr;
    m_NullTextureDescriptor = {};
    m_MainSRVHeap = nullptr;
    m_CBVHeaps = nullptr;
    m_ClearColour = DirectX::XMFLOAT4(0.424f, 0.725f, 0.788f, 1.0f);
    m_IsInitialised = false;
    m_DXGIFactory = nullptr;
    m_Device = nullptr;
    m_InfoQueue = nullptr;
    m_Fence = nullptr;
    m_RTVDescriptorHeapSize = 0;
    m_DSVDescriptorHeapSize = 0;
    m_CBVSRVDescriptorHeapSize = 0;
    m_m4xMSAAQuality = 0;
    m_CommandQueue = nullptr;
    m_CommandList = nullptr;
    m_CommandAllocator = nullptr;
    m_SwapChain = nullptr;
    m_CurrentBackbufferIndex = 0;
    m_WindowHandle = NULL;
    m_RTVHeap = nullptr;
    m_DSVHeap = nullptr;
    m_CurrentFenceIndex = 0;
    m_WindowHeight = 0;
    m_WindowWidth = 0;
    m_SwapchainBuffers = nullptr;
    m_Viewport = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_ScissorRect = {};
    m_DepthStencilBuffer = nullptr;
    m_ColourOnlyPipeline = nullptr;
    m_ColourOnlyVertexShaderBlob = nullptr;
    m_ColourOnlyPixelShaderBlob = nullptr;
    m_DefaultPixelShaderBlob = nullptr;
    m_DefaultVertexShaderBlob = nullptr;
    m_DefaultPipeline = nullptr;
    m_RootSignature = nullptr;
    m_PerObjectMatrixData = nullptr;
    m_PerObjectTextureData = nullptr;
    m_CBVHeaps = new ID3D12DescriptorHeap * [m_SwapChainBufferCount];
    m_ConstantBufferAddressArray = new char*[m_SwapChainBufferCount];
    m_ConstantBufferGPUUploaderArray = new ID3D12Resource*[m_SwapChainBufferCount];
    m_LightBufferAddressArray = new char* [m_SwapChainBufferCount];
    m_LightBufferGPUUploaderArray = new ID3D12Resource * [m_SwapChainBufferCount];

    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        m_ConstantBufferGPUUploaderArray[i] = nullptr;
        m_ConstantBufferAddressArray[i] = nullptr;
        m_LightBufferAddressArray[i] = nullptr;
        m_LightBufferGPUUploaderArray[i] = nullptr;
        m_CBVHeaps[i] = nullptr;
    }

    DirectX::XMStoreFloat4x4(&m_PerspProjectionMatrix, DirectX::XMMatrixIdentity());
    DirectX::XMStoreFloat4x4(&m_OrthoProjectionMatrix, DirectX::XMMatrixIdentity());
}

Renderer::~Renderer()
{
    CUSTOM_ASSERT(IsInitialised() == false);

    if (m_ConstantBufferGPUUploaderArray != nullptr)
    {
        delete[] m_ConstantBufferGPUUploaderArray;
        m_ConstantBufferGPUUploaderArray = nullptr;
    }

    if (m_ConstantBufferAddressArray != nullptr)
    {
        delete[] m_ConstantBufferAddressArray;
        m_ConstantBufferAddressArray = nullptr;
    }

    if (m_LightBufferGPUUploaderArray != nullptr)
    {
        delete[] m_LightBufferGPUUploaderArray;
        m_LightBufferGPUUploaderArray = nullptr;
    }

    if (m_LightBufferAddressArray != nullptr)
    {
        delete[] m_LightBufferAddressArray;
        m_LightBufferAddressArray = nullptr;
    }

    if (m_CBVHeaps != nullptr)
    {
        delete[] m_CBVHeaps;
        m_CBVHeaps = nullptr;
    }
}

const bool& Renderer::IsInitialised() const
{
    return m_IsInitialised;
}

ID3D12CommandQueue* Renderer::GetCommandQueue()
{
    CUSTOM_ASSERT(m_IsInitialised);
    return m_CommandQueue;
}

const ID3D12CommandQueue* Renderer::GetCommandQueue() const
{
    CUSTOM_ASSERT(m_IsInitialised);
    return m_CommandQueue;
}

HRESULT Renderer::ResetCommandList()
{
    return m_CommandList->Reset(m_CommandAllocator, m_DefaultPipeline);
}

ID3D12GraphicsCommandList* Renderer::GetCommandList()
{
    CUSTOM_ASSERT(m_IsInitialised);
    return m_CommandList;
}

const ID3D12GraphicsCommandList* Renderer::GetCommandList() const
{
    CUSTOM_ASSERT(m_IsInitialised);
    return m_CommandList;
}

const ID3D12Device* Renderer::GetDevice() const
{
    CUSTOM_ASSERT(m_IsInitialised);
    return m_Device;
}

ID3D12Device* Renderer::GetDevice()
{
    CUSTOM_ASSERT(m_IsInitialised);
    return m_Device;
}

bool Renderer::Initialise(Renderer& renderer, const HWND& windowHandle)
{
    if (renderer.IsInitialised())
    {
        Debug::LogWarning("Calling initialise on a renderer object that already exists.\n");
        return false;
    }

    if (windowHandle == NULL)
    {
        Debug::LogWarning("Passing an invalid window handle.\n");
        return false;
    }

    HRESULT hr = S_OK;
    renderer.m_IsInitialised = true;
    renderer.m_WindowHandle = windowHandle;

    hr = renderer.CreateDeviceAndFactory();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr);

    hr = renderer.CreateFence();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr);

    hr = renderer.DetermineMultisamplingDetails();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr);

    hr = renderer.CreateCommandObjects();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateSwapChain();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateDescriptorHeaps();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateRenderTargetViews();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateDepthStencilBuffer();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.UpdateViewportAndScissorRect();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateInputLayout();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateConstantBufferHeap();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateConstantBuffers();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateRootSignatureAndDescriptorTable();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.FindAndCreateShaders();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateGraphicsPipelines();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    hr = renderer.CreateNullDescriptors();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

    renderer.m_PerObjectMatrixData = new PerObjectMatrixData();
    renderer.m_PerObjectTextureData = new PerObjectTextureData();

    if (renderer.IsInitialised() == false)
    {
        Debug::LogSevere("Failed to initialise renderer.\n");
    }

    return renderer.m_IsInitialised;
}

void Renderer::Shutdown(Renderer& renderer)
{
    if (!renderer.IsInitialised())
    {
        Debug::LogWarning("Calling shutdown on a renderer object that doesn't exist.");
        return;
    }

    if (renderer.m_PerObjectMatrixData != nullptr)
    {
        delete renderer.m_PerObjectMatrixData;
        renderer.m_PerObjectMatrixData = nullptr;
    }

    if (renderer.m_PerObjectTextureData != nullptr)
    {
        delete renderer.m_PerObjectTextureData;
        renderer.m_PerObjectTextureData = nullptr;
    }

    renderer.DestroyNullDescriptors();
    renderer.DestroyGraphicsPipelines();
    renderer.DestroyLoadedShaders();
    renderer.DestroyRootSignatureAndDescriptorTable();
    renderer.DestroyConstantBuffers();
    renderer.DestroyConstantBufferHeap();
    renderer.DestroyInputLayout();
    renderer.DestroyDepthStencilBuffer();
    renderer.DestroyRenderTargetViews();
    renderer.DestroyDescriptorHeaps();
    renderer.DestroySwapChain();
    renderer.DestroyCommandObjects();
    renderer.ClearMultisamplingDetails();
    renderer.DestroyFence();
    renderer.DestroyDeviceAndFactory();

    renderer.m_WindowHandle = NULL;
}

const int& Renderer::GetWindowWidth() const
{
    return m_WindowWidth;
}

const int& Renderer::GetWindowHeight() const
{
    return m_WindowHeight;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& Renderer::GetNullTextureDescriptor() const
{
    return m_NullTextureDescriptor;
}

HRESULT Renderer::AssignTextureToSlot(const int& index, Texture* texture)
{
    if (m_PerObjectTextureData == nullptr)
    {
        return E_POINTER;
    }

    if (index >= MAX_TEXTURES_PER_SHADER || index < 0)
    {
        return E_BOUNDS;
    }

    m_PerObjectTextureData->TextureSlotIDs[index] = 0;

    if (texture != nullptr)
    {
        m_PerObjectTextureData->TextureSlotIDs[index] = texture->GetID();
    }

    UploadPushConstants();

    return S_OK;
}

const DirectX::XMFLOAT4X4& Renderer::GetPerspectiveProjectionMatrix() const
{
    return m_PerspProjectionMatrix;
}

DirectX::XMFLOAT4X4& Renderer::GetPerspectiveProjectionMatrix()
{
    return m_PerspProjectionMatrix;
}

const DirectX::XMFLOAT4X4& Renderer::GetOrthographicProjectionMatrix() const
{
    return m_OrthoProjectionMatrix;
}

DirectX::XMFLOAT4X4& Renderer::GetOrthographicProjectionMatrix()
{
    return m_OrthoProjectionMatrix;
}

const DirectX::XMFLOAT4X4 Renderer::GetViewMatrix() const
{
    return m_Camera.GetViewMatrix();
}

HRESULT Renderer::ResizeSwapchain(const int& newWidth, const int& newHeight)
{
    CUSTOM_ASSERT(m_IsInitialised);

    if (m_SwapChain == nullptr)
    {
        Debug::LogFatal("Swapchain is invalid for resizing.\n");
        return E_FAIL;
    }

    DestroyRenderTargetViews();
    DestroyDepthStencilBuffer();

    //Passing unknown format to retain the same format as the current buffers.
    HRESULT result = m_SwapChain->ResizeBuffers(m_SwapChainBufferCount, newWidth, newHeight, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    if (FAILED(result))
    {
        Debug::LogSevere("Serious failure while resizing swapchain buffers\n");
        return result;
    }

    m_WindowWidth = newWidth;
    m_WindowHeight = newHeight;

    result = CreateRenderTargetViews();

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to recreate render target views after swapchain resizing.\n");
        return result;
    }

    result = CreateDepthStencilBuffer();

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to recreate depth stencil view after swapchain resizing.\n");
        return result;
    }

    m_CurrentBackbufferIndex = 0;

    result = UpdateViewportAndScissorRect();

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to update viewport or scissor rect during swapchain resize.\n");
        return result;
    }

    return result;
}

void Renderer::BeginOrthographicDrawing(ConstantBuffer& constantBuffer)
{
    DirectX::XMFLOAT4X4 vm = GetViewMatrix();

    DirectX::XMStoreFloat4x4(&m_PerObjectMatrixData->World, DirectX::XMMatrixIdentity());
    DirectX::XMStoreFloat4x4(&m_PerObjectMatrixData->ViewProjection, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&vm) * DirectX::XMLoadFloat4x4(&GetOrthographicProjectionMatrix())));

    UploadPushConstants();
    
    UpdateConstantBuffer(constantBuffer);

    m_CommandList->SetPipelineState(m_OrthoPipeline);
}

void Renderer::TestTwoDimensionDraw(Model* model, Texture* texture)
{
    if (texture != nullptr)
    {
        AssignTextureToSlot(0, texture);
    }

    if (model != nullptr)
    {
        model->Render(*this);
    }
}

HRESULT Renderer::CreateDeviceAndFactory()
{
    HRESULT result = E_FAIL;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
    UINT factoryFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
    //Enabling D3D12 Debug layer.
    ID3D12Debug* debug;
    result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));

    if (FAILED(result))
    {
        Debug::LogWarning("Failed to enable debug layer within debug build.\n");
    }

    if (debug != nullptr && result == S_OK)
    {
        debug->EnableDebugLayer();
        // Enable additional debug layers.
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    result = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_DXGIFactory));
    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create D3D12 Factorys.\n");
        return result;
    }

    //Create hardware device
    result = D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&m_Device));
    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create D3D12 Device.\n");
        return result;
    }

    m_Device->SetName(L"Graphics Device");

#if defined(DEBUG) || defined(_DEBUG)
    m_Device->QueryInterface(IID_PPV_ARGS(&m_InfoQueue));
    m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
#endif

    //Cache descriptor sizes for later.
    m_RTVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_DSVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CBVSRVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return result;
}

void Renderer::DestroyDeviceAndFactory()
{
    if (m_InfoQueue != nullptr)
    {
        m_InfoQueue->Release();
        m_InfoQueue = nullptr;
    }

    if (m_Device != nullptr)
    {
        m_Device->Release();
        m_Device = nullptr;
    }

    if (m_DXGIFactory != nullptr)
    {
        m_DXGIFactory->Release();
        m_DXGIFactory = nullptr;
    }
}

HRESULT Renderer::CreateFence()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr)
        return E_POINTER;

    D3D12_FENCE_FLAGS fenceFlag = D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE;
    result = m_Device->CreateFence(0, fenceFlag, IID_PPV_ARGS(&m_Fence));

    m_Fence->SetName(L"Default Fence");

    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create fence from ID3D12Device.\n");
        return result;
    }

    return result;
}

void Renderer::DestroyFence()
{
    if (m_Fence != nullptr)
    {
        m_Fence->Release();
        m_Fence = nullptr;
    }

    m_RTVDescriptorHeapSize = 0;
    m_DSVDescriptorHeapSize = 0;
    m_CBVSRVDescriptorHeapSize = 0;
}

HRESULT Renderer::DetermineMultisamplingDetails()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr)
        return E_POINTER;

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleQualityLevels{};
    multisampleQualityLevels.Format = m_BackbufferFormat;
    multisampleQualityLevels.SampleCount = 4;
    multisampleQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    multisampleQualityLevels.NumQualityLevels = 0;

    result = m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &multisampleQualityLevels, sizeof(multisampleQualityLevels));

    if (FAILED(result) || multisampleQualityLevels.NumQualityLevels <= 0)
    {
        Debug::LogFatal("Failed to locate multisampling quality levels from ID3D12Device.\n");
        return result;
    }

    m_m4xMSAAQuality = multisampleQualityLevels.NumQualityLevels;

    return result;
}

void Renderer::ClearMultisamplingDetails()
{
    m_m4xMSAAQuality = 0;
}

HRESULT Renderer::CreateCommandObjects()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr)
        return E_POINTER;

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
    commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
    
    result = m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_CommandQueue));

    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create command queue from ID3D12Device.\n");
        return result;
    }

    m_CommandQueue->SetName(L"Command Queue");

    result = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));

    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create command allocator from ID3D12Device.\n");
        return result;
    }

    m_CommandAllocator->SetName(L"Command Allocator");

    result = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator, nullptr, IID_PPV_ARGS(&m_CommandList));

    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create command list from ID3D12Device.\n");
        return result;
    }
    
    m_CommandList->SetName(L"Command List");

    return result;
}

void Renderer::DestroyCommandObjects()
{
    if (m_CommandList != nullptr)
    {
        m_CommandList->Release();
        m_CommandList = nullptr;
    }

    if (m_CommandAllocator != nullptr)
    {
        m_CommandAllocator->Release();
        m_CommandAllocator = nullptr;
    }

    if (m_CommandQueue != nullptr)
    {
        m_CommandQueue->Release();
        m_CommandQueue = nullptr;
    }
}

HRESULT Renderer::CreateSwapChain()
{
    HRESULT result = E_FAIL;

    if (m_DXGIFactory == nullptr || m_CommandQueue == nullptr)
        return E_POINTER;

    DXGI_SWAP_CHAIN_FLAG swapChainFlags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    DXGI_MODE_DESC bufferDesc{};
    bufferDesc.Width = 0;
    bufferDesc.Height = 0;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = m_BackbufferFormat;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    RECT rect{};
    GetWindowRect(m_WindowHandle, &rect);
    m_WindowWidth = rect.right - rect.left;
    m_WindowHeight = rect.bottom - rect.top;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_WindowWidth;
    swapChainDesc.Height = m_WindowHeight;
    swapChainDesc.Format = m_BackbufferFormat;
    swapChainDesc.Stereo = false;
    swapChainDesc.BufferCount = m_SwapChainBufferCount;
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = swapChainFlags;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    result = m_DXGIFactory->CreateSwapChainForHwnd(
        m_CommandQueue,
        m_WindowHandle,
        &swapChainDesc,
        nullptr, nullptr, &m_SwapChain);

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create swap chain.\n");
        return result;
    }

    return result;
}

void Renderer::DestroySwapChain()
{
    if (m_SwapChain != nullptr)
    {
        m_SwapChain->Release();
        m_SwapChain = nullptr;
    }
}

HRESULT Renderer::CreateDescriptorHeaps()
{
    HRESULT result = E_FAIL;
    
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDescriptorHeapDesc.NumDescriptors = m_SwapChainBufferCount;
    rtvDescriptorHeapDesc.NodeMask = 0;
    rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    result = m_Device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&m_RTVHeap));

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create RTV descriptor heap.\n");
        return result;
    }

    m_RTVHeap->SetName(L"Render Target Heap");

    D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
    dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvDescriptorHeapDesc.NumDescriptors = 1;
    dsvDescriptorHeapDesc.NodeMask = 0;
    dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_Device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&m_DSVHeap));

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create DSV descriptor heap.\n");
        return result;
    }

    m_DSVHeap->SetName(L"Depth Stencil Heap");

    D3D12_DESCRIPTOR_HEAP_DESC mainSRVDescriptorHeapDesc{};
    mainSRVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    mainSRVDescriptorHeapDesc.NumDescriptors = MAX_LOADABLE_TEXTURES;
    mainSRVDescriptorHeapDesc.NodeMask = 0;
    mainSRVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_Device->CreateDescriptorHeap(&mainSRVDescriptorHeapDesc, IID_PPV_ARGS(&m_MainSRVHeap));

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create main SRV descriptor heap.\n");
        return result;
    }

    m_MainSRVHeap->SetName(L"Main CBV/SRV/UAV Heap");

    return result;
}

void Renderer::DestroyDescriptorHeaps()
{
    if (m_RTVHeap != nullptr)
    {
        m_RTVHeap->Release();
        m_RTVHeap = nullptr;
    }

    if (m_DSVHeap != nullptr)
    {
        m_DSVHeap->Release();
        m_DSVHeap = nullptr;
    }

    if (m_MainSRVHeap != nullptr)
    {
        m_MainSRVHeap->Release();
        m_MainSRVHeap = nullptr;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::GetCurrentBackbufferView() const
{
    CUSTOM_ASSERT((m_RTVHeap != nullptr));
    D3D12_CPU_DESCRIPTOR_HANDLE handle = D3D12_CPU_DESCRIPTOR_HANDLE();
    handle.ptr = m_RTVHeap->GetCPUDescriptorHandleForHeapStart().ptr + (m_CurrentBackbufferIndex * m_RTVDescriptorHeapSize);
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::GetDepthStencilBufferView() const
{
    CUSTOM_ASSERT((m_DSVHeap != nullptr));
    return m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

HRESULT Renderer::CreateRenderTargetViews()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr || m_SwapChain == nullptr || m_RTVHeap == nullptr)
    {
        Debug::LogSevere("Tried to create render target views with invalid pointer value.\n");
        return E_POINTER;
    }
    
    if (m_SwapchainBuffers != nullptr)
    {
        Debug::LogSevere("Trying to create swap chain buffer list when they already exist.\n");
        return E_POINTER;
    }

    m_SwapchainBuffers = new ID3D12Resource*[m_SwapChainBufferCount];

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
    
    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        result = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapchainBuffers[i]));

        if (FAILED(result))
        {
            Debug::LogSevere("Failed to get swap chain buffer %ui from swap chain.\n", i);
            return result;
        }

        m_Device->CreateRenderTargetView(m_SwapchainBuffers[i], nullptr, rtvHandle);
        std::wstring n = L"Swapchain Buffer" + std::to_wstring(i);
        m_SwapchainBuffers[i]->SetName(n.c_str());

        rtvHandle.ptr += m_RTVDescriptorHeapSize;
    }

    return result;
}

void Renderer::DestroyRenderTargetViews()
{
    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        m_SwapchainBuffers[i]->Release();
        m_SwapchainBuffers[i] = nullptr;
    }

    delete[] m_SwapchainBuffers;
    m_SwapchainBuffers = nullptr;
}

HRESULT Renderer::CreateDepthStencilBuffer()
{
    HRESULT result = E_FAIL;

    if (m_Device == nullptr || m_CommandList == nullptr)
    {
        Debug::LogSevere("Tried to create depth stencil buffer with an invalid device.\n");
        return E_POINTER;
    }

    D3D12_RESOURCE_DESC depthStencilDesc{};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_WindowWidth;
    depthStencilDesc.Height = m_WindowHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = m_DepthStencilBufferFormat;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Color[0] = 1.0f;
    clearValue.Color[1] = 1.0f;
    clearValue.Color[2] = 1.0f;
    clearValue.Color[3] = 1.0f;
    clearValue.Format = m_DepthStencilBufferFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    result = m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_DepthStencilBuffer));

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create commited device resource for depth stencil buffer\n.");
        return result;
    }

    m_Device->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, GetDepthStencilBufferView());

    m_DepthStencilBuffer->SetName(L"Depth Stencil Buffer");

    return result;
}

void Renderer::DestroyDepthStencilBuffer()
{
    if (m_DepthStencilBuffer != nullptr)
    {
        m_DepthStencilBuffer->Release();
        m_DepthStencilBuffer = nullptr;
    }        
}

HRESULT Renderer::UpdateViewportAndScissorRect()
{
    HRESULT result = E_FAIL;

    if (m_CommandList == nullptr)
    {
        Debug::LogSevere("Tried to set a viewport with an invalid command list.\n");
        return E_POINTER;
    }

    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;
    m_Viewport.Width = (float)m_WindowWidth;
    m_Viewport.Height = (float)m_WindowHeight;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    m_ScissorRect.left = 0;
    m_ScissorRect.top = 0;
    m_ScissorRect.right = m_WindowWidth;
    m_ScissorRect.bottom = m_WindowHeight;
    
    DirectX::XMStoreFloat4x4(&m_PerspProjectionMatrix, (DirectX::XMMatrixPerspectiveFovLH(90.0f * (3.1415926535f / 180.0f), (float)m_WindowWidth / (float)m_WindowHeight, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE)));
    DirectX::XMStoreFloat4x4(&m_OrthoProjectionMatrix, (DirectX::XMMatrixOrthographicLH((float)m_WindowWidth, (float)m_WindowHeight, FLT_EPSILON, DEFAULT_FAR_PLANE)));

    return S_OK;
}

HRESULT Renderer::FlushCommandQueue()
{
    HRESULT result = E_FAIL;
    CUSTOM_ASSERT(m_IsInitialised);
    m_CurrentFenceIndex++;
    result = m_CommandQueue->Signal(m_Fence, m_CurrentFenceIndex);

    if (FAILED(result))
    {
        Debug::LogSevere("Command Queue failed to signal fence.\n");
        return result;
    }

    if (m_Fence->GetCompletedValue() < m_CurrentFenceIndex)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, "Fence", 0, EVENT_ALL_ACCESS);
        result = m_Fence->SetEventOnCompletion(m_CurrentFenceIndex, eventHandle);

        if (FAILED(result))
        {
            Debug::LogSevere("Failed to set signal event for fence.\n");
            return result;
        }

        if (eventHandle != NULL)
        {
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    return result;
}

HRESULT Renderer::CreateConstantBuffers()
{
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    ConstantBuffer emptyCb = ConstantBuffer();
    LightBuffer emptyLb = LightBuffer();

    HRESULT result = E_FAIL;

    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        //Creating GPU upload buffer.
        CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RANGE readRange(0, 0);

        //Creating per-frame Constant Buffer
        CD3DX12_RESOURCE_DESC cbResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer));

        result = m_Device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &cbResourceDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_ConstantBufferGPUUploaderArray[i]));

        if (FAILED(result))
        {
            Debug::LogSevere("Failed to create commited resource for constant buffer's GPU upload buffer.\n");
            return result;
        }

        std::wstring name = L"Constant Buffer GPU Upload Heap " + std::to_wstring(i);
        m_ConstantBufferGPUUploaderArray[i]->SetName(name.c_str());
        
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
        cbvDesc.BufferLocation = m_ConstantBufferGPUUploaderArray[i]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;

        m_Device->CreateConstantBufferView(&cbvDesc, m_CBVHeaps[i]->GetCPUDescriptorHandleForHeapStart());

        result = m_ConstantBufferGPUUploaderArray[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_ConstantBufferAddressArray[i]));

        if (FAILED(result) || m_ConstantBufferAddressArray[i] == nullptr)
        {
            Debug::LogSevere("Failed to map constant buffer's GPU upload heap address.\n");
            return result;
        }

        memcpy(m_ConstantBufferAddressArray[i], &emptyCb, sizeof(ConstantBuffer));

        //--------------------------------------------------------------------//

        //Creating per-frame Light Buffer
        CD3DX12_RESOURCE_DESC lbResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(LightBuffer));

        result = m_Device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &lbResourceDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_LightBufferGPUUploaderArray[i]));

        if (FAILED(result))
        {
            Debug::LogSevere("Failed to create committed resource for light buffer's GPU upload buffer.\n");
            return result;
        }

        name = L"Light Buffer GPU Upload Heap " + std::to_wstring(i);
        m_LightBufferGPUUploaderArray[i]->SetName(name.c_str());

        D3D12_CONSTANT_BUFFER_VIEW_DESC lbvDesc{};
        lbvDesc.BufferLocation = m_LightBufferGPUUploaderArray[i]->GetGPUVirtualAddress();
        lbvDesc.SizeInBytes = (sizeof(LightBuffer) + 255) & ~255;
        
        CD3DX12_CPU_DESCRIPTOR_HANDLE lbHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CBVHeaps[i]->GetCPUDescriptorHandleForHeapStart());
        lbHandle.Offset(1, m_CBVSRVDescriptorHeapSize);
        m_Device->CreateConstantBufferView(&lbvDesc, lbHandle);

        result = m_LightBufferGPUUploaderArray[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_LightBufferAddressArray[i]));

        if (FAILED(result) || m_LightBufferAddressArray[i] == nullptr)
        {
            Debug::LogSevere("Failed to map light buffer's GPU upload heap address.\n");
            return result;
        }

        memcpy(m_LightBufferAddressArray[i], &emptyLb, sizeof(LightBuffer));
    }

    return S_OK;
}

void Renderer::DestroyConstantBuffers()
{
    CD3DX12_RANGE readRange(0, 0);

    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        if (m_ConstantBufferGPUUploaderArray[i] != nullptr)
        {
            m_ConstantBufferGPUUploaderArray[i]->Unmap(0, &readRange);
            m_ConstantBufferGPUUploaderArray[i]->Release();
            m_ConstantBufferGPUUploaderArray[i] = nullptr;
        }

        if (m_ConstantBufferAddressArray[i] != nullptr)
        {
            m_ConstantBufferAddressArray[i] = nullptr;
        }

        if (m_LightBufferGPUUploaderArray[i] != nullptr)
        {
            m_LightBufferGPUUploaderArray[i]->Unmap(0, &readRange);
            m_LightBufferGPUUploaderArray[i]->Release();
            m_LightBufferGPUUploaderArray[i] = nullptr;
        }

        if (m_LightBufferAddressArray[i] != nullptr)
        {
            m_LightBufferAddressArray[i] = nullptr;
        }
    }
}

HRESULT Renderer::CreateConstantBufferHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = 2;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    HRESULT result = E_FAIL;

    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        result = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CBVHeaps[i]));

        if (FAILED(result))
        {
            Debug::LogSevere("Failed to create a constant buffer descriptor heap.\n");
            break;
        }

        std::wstring name = L"Constant Buffer Heap " + std::to_wstring(i);
        m_CBVHeaps[i]->SetName(name.c_str());
    }

    return result;
}

void Renderer::DestroyConstantBufferHeap()
{
    for (size_t i = 0; i < m_SwapChainBufferCount; i++)
    {
        if (m_CBVHeaps[i] != nullptr)
        {
            m_CBVHeaps[i]->Release();
            m_CBVHeaps[i] = nullptr;
        }        
    }
}

HRESULT Renderer::CreateRootSignatureAndDescriptorTable()
{
    D3D12_ROOT_PARAMETER slotRootParameters[5]{};

    //CBV
    D3D12_ROOT_DESCRIPTOR perFrameConstantBufferDescriptor{};
    perFrameConstantBufferDescriptor.RegisterSpace = 0;
    perFrameConstantBufferDescriptor.ShaderRegister = 0;
    slotRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    slotRootParameters[0].Descriptor = perFrameConstantBufferDescriptor;
    slotRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    //Push Constants
    D3D12_ROOT_CONSTANTS matrixDataRootConstants{};
    matrixDataRootConstants.Num32BitValues = sizeof(PerObjectMatrixData) / sizeof(UINT32);
    matrixDataRootConstants.RegisterSpace = 0;
    matrixDataRootConstants.ShaderRegister = 1;
    D3D12_ROOT_DESCRIPTOR matrixDataDescriptor{};
    matrixDataDescriptor.RegisterSpace = matrixDataRootConstants.RegisterSpace;
    matrixDataDescriptor.ShaderRegister = matrixDataRootConstants.ShaderRegister;
    slotRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    slotRootParameters[1].Descriptor = matrixDataDescriptor;
    slotRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
    slotRootParameters[1].Constants = matrixDataRootConstants;

    D3D12_ROOT_CONSTANTS textureDataRootConstants{};
    textureDataRootConstants.Num32BitValues = sizeof(PerObjectTextureData) / sizeof(UINT32);
    textureDataRootConstants.RegisterSpace = 0;
    textureDataRootConstants.ShaderRegister = 2;
    D3D12_ROOT_DESCRIPTOR textureDataDescriptor{};
    textureDataDescriptor.RegisterSpace = textureDataRootConstants.RegisterSpace;
    textureDataDescriptor.ShaderRegister = textureDataRootConstants.ShaderRegister;
    slotRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    slotRootParameters[2].Descriptor = textureDataDescriptor;
    slotRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
    slotRootParameters[2].Constants = textureDataRootConstants;

    //Lighting Constant Buffer
    D3D12_ROOT_DESCRIPTOR lightingConstantBufferDescriptor{};
    lightingConstantBufferDescriptor.RegisterSpace = 0;
    lightingConstantBufferDescriptor.ShaderRegister = 3;
    slotRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    slotRootParameters[3].Descriptor = lightingConstantBufferDescriptor;
    slotRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    //SRV Table
    D3D12_DESCRIPTOR_RANGE descriptorTableRange[1]{};
    descriptorTableRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorTableRange[0].NumDescriptors = MAX_LOADABLE_TEXTURES;
    descriptorTableRange[0].BaseShaderRegister = 0;
    descriptorTableRange[0].RegisterSpace = 0;
    descriptorTableRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable{};
    descriptorTable.NumDescriptorRanges = _countof(descriptorTableRange);
    descriptorTable.pDescriptorRanges = &descriptorTableRange[0];
    slotRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    slotRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    slotRootParameters[4].DescriptorTable = descriptorTable;

    D3D12_STATIC_SAMPLER_DESC staticSamplerDesc[1]{};
    staticSamplerDesc[0].Filter = D3D12_FILTER::D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    staticSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSamplerDesc[0].MipLODBias = 0;
    staticSamplerDesc[0].MaxAnisotropy = 0;
    staticSamplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    staticSamplerDesc[0].MinLOD = 0.0f;
    staticSamplerDesc[0].MaxLOD = FLT_MAX;
    staticSamplerDesc[0].ShaderRegister = 0;
    staticSamplerDesc[0].RegisterSpace = 0;
    staticSamplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Init(
        _countof(slotRootParameters), slotRootParameters,
        _countof(staticSamplerDesc), &staticSamplerDesc[0],
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    ID3DBlob* rootSigBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create serialised root signature.\n");
        OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));

        if (errorBlob != nullptr)
        {
            errorBlob->Release();
            errorBlob = nullptr;
        }

        return result;
    }

    if (errorBlob != nullptr)
    {
        errorBlob->Release();
        errorBlob = nullptr;
    }

    result = m_Device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create root signature.\n");

        if (rootSigBlob != nullptr)
        {
            rootSigBlob->Release();
            rootSigBlob = nullptr;
        }

        return result;
    }

    if (rootSigBlob != nullptr)
    {
        rootSigBlob->Release();
        rootSigBlob = nullptr;
    }

    m_RootSignature->SetName(L"Root Signature");

    m_CommandList->SetGraphicsRootSignature(m_RootSignature);

    return S_OK;
}

void Renderer::DestroyRootSignatureAndDescriptorTable()
{
    if (m_RootSignature != nullptr)
    {
        m_RootSignature->Release();
        m_RootSignature = nullptr;
    }
}

HRESULT Renderer::ReadShaderData(const std::string& filename, ID3DBlob*& targetBlob)
{
    std::filesystem::path path = filename;

    if (std::filesystem::exists(path) == false)
    {
        Debug::LogSevere("Failed to locate shader file '%s'\n", filename.c_str());
        return E_INVALIDARG;
    }

    if (targetBlob != nullptr)
    {
        Debug::LogSevere("Trying to load shader into already loaded blob.\n");
        return E_POINTER;
    }

    //Load file.
    std::ifstream file(filename, std::ios::binary);
    //Seek end
    file.seekg(0, std::ios::end);
    //Get size from current position
    int size = file.tellg();
    //Go back to the start for read.
    file.seekg(0, std::ios::beg);

    HRESULT result = D3DCreateBlob(size, &targetBlob);
    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create blob during shader object creation.\n");
        return E_POINTER;
    }

    file.read((char*)targetBlob->GetBufferPointer(), size);
    file.close();

    return S_OK;
}

HRESULT Renderer::FindAndCreateShaders()
{
    HRESULT result = E_UNEXPECTED;

    result = ReadShaderData("PS_Standard.cso", m_DefaultPixelShaderBlob);

    if (FAILED(result))
    {
        //Error message displayed in function.
        return result;
    }

    result = ReadShaderData("VS_Standard.cso", m_DefaultVertexShaderBlob);

    if (FAILED(result))
    {
        //Error message displayed in function.
        return result;
    }

    result = ReadShaderData("PS_ColourOnly.cso", m_ColourOnlyPixelShaderBlob);

    if (FAILED(result))
    {
        //Error message displayed in function.
        return result;
    }

    result = ReadShaderData("VS_ColourOnly.cso", m_ColourOnlyVertexShaderBlob);

    if (FAILED(result))
    {
        //Error message displayed in function.
        return result;
    }

    result = ReadShaderData("PS_DefaultOrtho.cso", m_DefaultOrthoPixelShaderBlob);

    if (FAILED(result))
    {
        //Error message displayed in function.
        return result;
    }

    result = ReadShaderData("VS_DefaultOrtho.cso", m_DefaultOrthoVertexShaderBlob);

    if (FAILED(result))
    {
        //Error message displayed in function.
        return result;
    }


    return result;
}

void Renderer::DestroyLoadedShaders()
{
    if (m_DefaultPixelShaderBlob != nullptr)
    {
        m_DefaultPixelShaderBlob->Release();
        m_DefaultPixelShaderBlob = nullptr;
    }
    
    if (m_DefaultVertexShaderBlob != nullptr)
    {
        m_DefaultVertexShaderBlob->Release();
        m_DefaultVertexShaderBlob = nullptr;
    }
    
    if (m_ColourOnlyPixelShaderBlob != nullptr)
    {
        m_ColourOnlyPixelShaderBlob->Release();
        m_ColourOnlyPixelShaderBlob = nullptr;
    }
    
    if (m_ColourOnlyVertexShaderBlob != nullptr)
    {
        m_ColourOnlyVertexShaderBlob->Release();
        m_ColourOnlyVertexShaderBlob = nullptr;
    }
}

HRESULT Renderer::CreateGraphicsPipelines()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
    memset(&pipelineStateDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    pipelineStateDesc.pRootSignature = m_RootSignature;
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pipelineStateDesc.SampleMask = UINT_MAX; 
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.NodeMask = 0;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = m_BackbufferFormat;
    pipelineStateDesc.DSVFormat = m_DepthStencilBufferFormat;
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleDesc.Quality = 0;

    HRESULT result = E_POINTER;
    if (m_DefaultVertexShaderBlob != nullptr && m_DefaultPixelShaderBlob != nullptr)
    {
        pipelineStateDesc.InputLayout.NumElements = m_DefaultInputLayout.size();
        pipelineStateDesc.InputLayout.pInputElementDescs = m_DefaultInputLayout.data();
        pipelineStateDesc.VS.pShaderBytecode = m_DefaultVertexShaderBlob->GetBufferPointer();
        pipelineStateDesc.VS.BytecodeLength = m_DefaultVertexShaderBlob->GetBufferSize();
        pipelineStateDesc.PS.pShaderBytecode = m_DefaultPixelShaderBlob->GetBufferPointer();
        pipelineStateDesc.PS.BytecodeLength = m_DefaultPixelShaderBlob->GetBufferSize();
        result = m_Device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&m_DefaultPipeline));
    }

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create default graphics pipeline state.\n");
        return result;
    }
    m_DefaultPipeline->SetName(L"Standard Graphics Pipeline");

    result = E_POINTER;
    if (m_ColourOnlyVertexShaderBlob != nullptr && m_ColourOnlyPixelShaderBlob != nullptr)
    {
        pipelineStateDesc.InputLayout.NumElements = m_ColourOnlyInputLayout.size();
        pipelineStateDesc.InputLayout.pInputElementDescs = m_ColourOnlyInputLayout.data();
        pipelineStateDesc.VS.pShaderBytecode = m_ColourOnlyVertexShaderBlob->GetBufferPointer();
        pipelineStateDesc.VS.BytecodeLength = m_ColourOnlyVertexShaderBlob->GetBufferSize();
        pipelineStateDesc.PS.pShaderBytecode = m_ColourOnlyPixelShaderBlob->GetBufferPointer();
        pipelineStateDesc.PS.BytecodeLength = m_ColourOnlyPixelShaderBlob->GetBufferSize();
        result = m_Device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&m_ColourOnlyPipeline));
    }

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create colour only graphics pipeline state.\n");
        return result;
    }

    m_ColourOnlyPipeline->SetName(L"Colour Vertex Graphics Pipeline");

    result = E_POINTER;
    if (m_DefaultOrthoVertexShaderBlob != nullptr && m_DefaultOrthoPixelShaderBlob != nullptr)
    {
        D3D12_RENDER_TARGET_BLEND_DESC transparencyBlend{};
        memset(&transparencyBlend, 0x0, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));
        transparencyBlend.BlendEnable = TRUE;
        transparencyBlend.LogicOpEnable = FALSE;
        transparencyBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        transparencyBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        transparencyBlend.BlendOp = D3D12_BLEND_OP_ADD;
        transparencyBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
        transparencyBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
        transparencyBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        transparencyBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;

        pipelineStateDesc.BlendState.RenderTarget[0] = transparencyBlend;
        pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
        pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
        pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pipelineStateDesc.InputLayout.NumElements = m_UIImageVertexInputLayout.size();
        pipelineStateDesc.InputLayout.pInputElementDescs = m_UIImageVertexInputLayout.data();
        pipelineStateDesc.VS.pShaderBytecode = m_DefaultOrthoVertexShaderBlob->GetBufferPointer();
        pipelineStateDesc.VS.BytecodeLength = m_DefaultOrthoVertexShaderBlob->GetBufferSize();
        pipelineStateDesc.PS.pShaderBytecode = m_DefaultOrthoPixelShaderBlob->GetBufferPointer();
        pipelineStateDesc.PS.BytecodeLength = m_DefaultOrthoPixelShaderBlob->GetBufferSize();
        pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        result = m_Device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&m_OrthoPipeline));
    }

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create Orthographic 2D Graphics pipeline state.\n");
        return result;
    }

    m_OrthoPipeline->SetName(L"Orthographic 2D Graphics Pipeline");


    return result;
}

void Renderer::DestroyGraphicsPipelines()
{
    if (m_ColourOnlyPipeline != nullptr)
    {
        m_ColourOnlyPipeline->Release();
        m_ColourOnlyPipeline = nullptr;
    }

    if (m_DefaultPipeline != nullptr)
    {
        m_DefaultPipeline->Release();
        m_DefaultPipeline = nullptr;
    }
}

HRESULT Renderer::CreateNullDescriptors()
{
    const DXGI_FORMAT textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2D(textureFormat);
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(GetMainSRVDescriptorHeapStartCPU(), 0, GetSRVDescriptorHeapSize());

    m_Device->CreateShaderResourceView(NULL, &srvDesc, srvCpuHandle);

    HRESULT result = ExecuteAndResetCommandList();

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to execute and reset command list during texture buffer creation.\n");
        return result;
    }

    m_NullTextureDescriptor = srvCpuHandle;

    return result;
}

void Renderer::DestroyNullDescriptors()
{
    m_NullTextureDescriptor = {};
}

UINT Renderer::GetSRVDescriptorHeapSize() const
{
    return m_CBVSRVDescriptorHeapSize;
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::GetMainSRVDescriptorHeapStartCPU() const
{
    CUSTOM_ASSERT((m_MainSRVHeap != nullptr));
    return m_MainSRVHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE Renderer::GetMainSRVDescriptorHeapStartGPU() const
{
    CUSTOM_ASSERT((m_MainSRVHeap != nullptr));
    return m_MainSRVHeap->GetGPUDescriptorHandleForHeapStart();
}

HRESULT Renderer::ExecuteAndResetCommandList()
{
    HRESULT result = E_FAIL;

    result = m_CommandList->Close();
    if (FAILED(result))
    {
        Debug::LogSevere("Failed to close command list during default buffer creation.\n");
        return result;
    }

    ID3D12CommandList* commandLists = { m_CommandList };
    m_CommandQueue->ExecuteCommandLists(1, &commandLists);

    result = FlushCommandQueue();
    if (FAILED(result))
    {
        Debug::LogSevere("Failed to flush command queue during default buffer creation.\n");
        return result;
    }

    result = ResetCommandList();
    if (FAILED(result))
    {
        Debug::LogSevere("Failed to reset command list during default buffer creation.\n");
        return result;
    }

    return result;
}

HRESULT Renderer::CreateDefaultBuffer(ID3D12Resource*& defaultBuffer, ID3D12Resource*& gpuUploadBuffer, const void* data, const size_t& sizeBytes)
{
    CUSTOM_ASSERT((defaultBuffer == nullptr));
    CUSTOM_ASSERT((gpuUploadBuffer == nullptr));
    CUSTOM_ASSERT((data != nullptr));
    CUSTOM_ASSERT(sizeBytes > 0);

    CD3DX12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC defaultBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer((UINT64)sizeBytes);

    //Creating Default Buffer
    HRESULT result = m_Device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE,
        &defaultBufferResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer));

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create commited resource for default buffer.\n");
        return result;
    }

    //Creating GPU upload buffer.
    CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    result = m_Device->CreateCommittedResource(
        &uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &defaultBufferResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&gpuUploadBuffer));

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to create commited resource for default buffer's GPU upload buffer.\n");
        return result;
    }

    //Describe data for copy into buffer.
    D3D12_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pData = data;
    subresourceData.RowPitch = sizeBytes;
    subresourceData.SlicePitch = sizeBytes;

    //Scheduling copy to the default buffer.
    //Copies CPU memory to immediate upload heap, then upload heap is copied into buffer by gpu.
    CD3DX12_RESOURCE_BARRIER toCopyTransition = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    m_CommandList->ResourceBarrier(1, &toCopyTransition);

    UpdateSubresources(m_CommandList,
        defaultBuffer, gpuUploadBuffer, 0, 
        0, 1, &subresourceData);

    CD3DX12_RESOURCE_BARRIER toReadTransition = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ);
    m_CommandList->ResourceBarrier(1, &toReadTransition);

    result = ExecuteAndResetCommandList();

    if (FAILED(result))
    {
        Debug::LogSevere("Failed to execute and reset command list during default buffer creation.\n");
        return result;
    }

    return S_OK;
}

HRESULT Renderer::CreateInputLayout()
{
    Vertex::GetElementDescription(m_DefaultInputLayout);
    ColourOnlyVertex::GetElementDescription(m_ColourOnlyInputLayout);
    UIImageVertex::GetElementDescription(m_UIImageVertexInputLayout);
    return S_OK;
}

void Renderer::DestroyInputLayout()
{
    m_DefaultInputLayout.clear();
    m_ColourOnlyInputLayout.clear();
    m_UIImageVertexInputLayout.clear();
}

const DirectX::XMFLOAT4& Renderer::GetClearColour() const
{
    return m_ClearColour;
}

void Renderer::SetClearColour(const DirectX::XMFLOAT4& newColour)
{
    m_ClearColour = newColour;
}

PerObjectMatrixData& Renderer::GetPerObjectMatrixData()
{
    return *m_PerObjectMatrixData;
}

PerObjectTextureData& Renderer::GetPerObjectTextureData()
{
    return *m_PerObjectTextureData;
}

void Renderer::UploadPushConstants()
{
    m_CommandList->SetGraphicsRoot32BitConstants(1, sizeof(PerObjectMatrixData) / sizeof(UINT32), m_PerObjectMatrixData, 0);
    m_CommandList->SetGraphicsRoot32BitConstants(2, sizeof(PerObjectTextureData) / sizeof(UINT32), m_PerObjectTextureData, 0);
}

HRESULT Renderer::UpdateWorldMatrix(const DirectX::XMFLOAT4X4& worldMatrix)
{
    CUSTOM_ASSERT(m_IsInitialised);
    m_PerObjectMatrixData->World = worldMatrix;
    UploadPushConstants();
    return S_OK;
}

HRESULT Renderer::UpdateMaterialBuffer(const Material& material)
{
    CUSTOM_ASSERT(m_IsInitialised);
    m_PerObjectTextureData->MaterialData = material;
    UploadPushConstants();
    return S_OK;
}

HRESULT Renderer::UpdateLightingBuffer(const LightBuffer& lb)
{
    CUSTOM_ASSERT(m_IsInitialised);

    if (m_LightBufferAddressArray[m_CurrentBackbufferIndex] != nullptr)
    {
        m_CommandList->SetGraphicsRootConstantBufferView(3, m_LightBufferGPUUploaderArray[m_CurrentBackbufferIndex]->GetGPUVirtualAddress());
        memcpy(m_LightBufferAddressArray[m_CurrentBackbufferIndex], &lb, sizeof(LightBuffer));
        return S_OK;
    }

    return E_FAIL;
}

HRESULT Renderer::UpdateConstantBuffer(const ConstantBuffer& cb)
{
    CUSTOM_ASSERT(m_IsInitialised);

    if (m_ConstantBufferAddressArray[m_CurrentBackbufferIndex] != nullptr)
    {
        m_CommandList->SetGraphicsRootConstantBufferView(0, m_ConstantBufferGPUUploaderArray[m_CurrentBackbufferIndex]->GetGPUVirtualAddress());
        memcpy(m_ConstantBufferAddressArray[m_CurrentBackbufferIndex], &cb, sizeof(ConstantBuffer));
        return S_OK;
    }

    return E_FAIL;
}

void Renderer::ClearFrame()
{
    CUSTOM_ASSERT(m_IsInitialised);
    
    if (FAILED(m_CommandAllocator->Reset()))
    {
        Debug::LogSevere("Failed to reset command allocator.\n");
        return;
    }

    if (FAILED(ResetCommandList()))
    {
        Debug::LogSevere("Failed to reset command allocator.\n");
        return;
    }

    m_CommandList->RSSetViewports(1, &m_Viewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

    D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = GetCurrentBackbufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvBufferHandle = GetDepthStencilBufferView();

    D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_CommandList->ResourceBarrier(1, &transition);

    float Colour[4] = { m_ClearColour.x, m_ClearColour.y, m_ClearColour.z, m_ClearColour.w };
    m_CommandList->ClearDepthStencilView(dsvBufferHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    transition = CD3DX12_RESOURCE_BARRIER::Transition(m_SwapchainBuffers[m_CurrentBackbufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_CommandList->ResourceBarrier(1, &transition);

    m_CommandList->ClearRenderTargetView(backBufferHandle, Colour, 0, nullptr);
    m_CommandList->OMSetRenderTargets(1, &backBufferHandle, true, &dsvBufferHandle);

    m_CommandList->SetGraphicsRootSignature(m_RootSignature);

    ID3D12DescriptorHeap* heaps[] = { m_MainSRVHeap };
    m_CommandList->SetDescriptorHeaps(_countof(heaps), heaps);  

    CD3DX12_GPU_DESCRIPTOR_HANDLE srvHeap(heaps[0]->GetGPUDescriptorHandleForHeapStart());
    m_CommandList->SetGraphicsRootDescriptorTable(4, srvHeap);
    m_CommandList->SetGraphicsRootSignature(m_RootSignature);

    m_CommandList->SetPipelineState(m_DefaultPipeline);

    m_CommandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::PresentFrame()
{
    CUSTOM_ASSERT(m_IsInitialised);

    D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(m_SwapchainBuffers[m_CurrentBackbufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_CommandList->ResourceBarrier(1, &transition);

    transition = CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PRESENT);
    m_CommandList->ResourceBarrier(1, &transition);

    if (FAILED(m_CommandList->Close()))
    {
        Debug::LogSevere("Failed to close command list.\n");
        return;
    }

    ID3D12CommandList* commandLists = { m_CommandList };
    m_CommandQueue->ExecuteCommandLists(1, &commandLists);

    HRESULT hr = m_SwapChain->Present(0, 0);

    if (FAILED(hr))
    {
        HRESULT removalReason = m_Device->GetDeviceRemovedReason();

        Debug::LogSevere("Failed to present swap chain.\n");
        return;
    }

    m_CurrentBackbufferIndex = (m_CurrentBackbufferIndex + 1) % m_SwapChainBufferCount;
    if (FAILED(FlushCommandQueue()))
    {
        Debug::LogSevere("Failed to flush command queue.\n");
        return;
    }
}
