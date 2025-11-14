#include "pch.h"
#include "Renderer.h"
#include <Graphics/DX12Includes.h>
#include <System/Debug.h>

#define FAILED_RETURN(hr) if(FAILED(hr)) return !FAILED(hr);

const int Renderer::m_SwapChainBufferCount = 2;

Renderer::Renderer()
{
    m_IsInitialised = false;
    m_DXGIFactory = nullptr;
    m_Device = nullptr;
    m_Fence = nullptr;
    m_RTVDescriptorHeapSize = 0;
    m_DSVDescriptorHeapSize = 0;
    m_CBVSRVDescriptorHeapSize = 0;
    m_BackbufferFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
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

    renderer.DestroyDescriptorHeaps();
    renderer.DestroySwapChain();
    renderer.DestroyCommandObjects();
    renderer.ClearMultisamplingDetails();
    renderer.DestroyFence();
    renderer.DestroyDeviceAndFactory();

    renderer.m_WindowHandle = NULL;
}

HRESULT Renderer::CreateDeviceAndFactory()
{
    HRESULT result = E_FAIL;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_2;
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

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = 0;  //Set to 0 so it can be taken from the window.
    swapChainDesc.Height = 0; //Set to 0 so it can be taken from the window.
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
