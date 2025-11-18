#include "pch.h"
#include "Renderer.h"
#include <System/Debug.h>

#define FAILED_RETURN(hr) if(FAILED(hr)) return !FAILED(hr);

const int Renderer::m_SwapChainBufferCount = 2;
const DXGI_FORMAT Renderer::m_BackbufferFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
const DXGI_FORMAT Renderer::m_DepthStencilBufferFormat = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;

Renderer::Renderer()
{
    m_IsInitialised = false;
    m_DXGIFactory = nullptr;
    m_Device = nullptr;
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
}

Renderer::~Renderer()
{
    CUSTOM_ASSERT(IsInitialised() == false);
}

const bool& Renderer::IsInitialised()
{
    return m_IsInitialised;
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

    hr = renderer.SetupInitialViewportAndScissorRect();
    renderer.m_IsInitialised &= SUCCEEDED(hr);
    FAILED_RETURN(hr)

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

    //Create hardware device
    result = D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&m_Device));
    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create D3D12 Device.\n");
        return result;
    }

    return result;
}

void Renderer::DestroyDeviceAndFactory()
{
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

    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create fence from ID3D12Device.\n");
        return result;
    }

    //Cache descriptor sizes for later.
    m_RTVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_DSVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CBVSRVDescriptorHeapSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleQualityLevels;
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

    result = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));

    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create command allocator from ID3D12Device.\n");
        return result;
    }

    result = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator, nullptr, IID_PPV_ARGS(&m_CommandList));

    if (FAILED(result))
    {
        Debug::LogFatal("Failed to create command list from ID3D12Device.\n");
        return result;
    }

    //Setting to closed as the first refernce to the command list will open it.
    if (m_CommandList)
    {
        m_CommandList->Close();
    }

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

    D3D12_CLEAR_VALUE clearValue;
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

    D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_CommandList->ResourceBarrier(1, &resourceBarrier);

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

HRESULT Renderer::SetupInitialViewportAndScissorRect()
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

    m_CommandList->RSSetViewports(1, &m_Viewport);

    m_ScissorRect.left = 0;
    m_ScissorRect.top = 0;
    m_ScissorRect.right = m_WindowWidth;
    m_ScissorRect.bottom = m_WindowHeight;
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

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

        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    return result;
}

HRESULT Renderer::CreateInputAssembly()
{
    //pg 206
    return E_NOTIMPL;
}

void Renderer::DestroyInputAssembly()
{
}

const DirectX::XMFLOAT4& Renderer::GetClearColour() const
{
    return m_ClearColour;
}

void Renderer::SetClearColour(const DirectX::XMFLOAT4& newColour)
{
    m_ClearColour = newColour;
}

void Renderer::ClearFrame()
{
    CUSTOM_ASSERT(m_IsInitialised);
    
    if (FAILED(m_CommandAllocator->Reset()))
    {
        Debug::LogSevere("Failed to reset command allocator.\n");
        return;
    }

    if (FAILED(m_CommandList->Reset(m_CommandAllocator, nullptr)))
    {
        Debug::LogSevere("Failed to reset command allocator.\n");
        return;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = GetCurrentBackbufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvBufferHandle = GetDepthStencilBufferView();
    D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(m_SwapchainBuffers[m_CurrentBackbufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    m_CommandList->RSSetViewports(1, &m_Viewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect); 
    float Colour[4] = { m_ClearColour.x, m_ClearColour.y, m_ClearColour.z, m_ClearColour.w };
    m_CommandList->ClearRenderTargetView(backBufferHandle, Colour, 0, nullptr);
    m_CommandList->ClearDepthStencilView(dsvBufferHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    m_CommandList->OMSetRenderTargets(1, &backBufferHandle, true, &dsvBufferHandle);
    m_CommandList->ResourceBarrier(1, &transition);
}

void Renderer::PresentFrame()
{
    CUSTOM_ASSERT(m_IsInitialised);
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
