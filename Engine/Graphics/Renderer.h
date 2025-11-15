#pragma once
#include <Graphics/DX12Includes.h>

class Renderer
{
private:
	int m_WindowWidth;
	int m_WindowHeight;
	bool m_IsInitialised;
	HWND m_WindowHandle;
	DirectX::XMFLOAT4 m_ClearColour;

	static const enum DXGI_FORMAT m_BackbufferFormat;
	static const enum DXGI_FORMAT m_DepthStencilBufferFormat;

	struct IDXGIFactory2* m_DXGIFactory;
	struct ID3D12Device* m_Device;
	HRESULT CreateDeviceAndFactory();
	void DestroyDeviceAndFactory();

	UINT m_RTVDescriptorHeapSize;
	UINT m_DSVDescriptorHeapSize;
	UINT m_CBVSRVDescriptorHeapSize;
	int m_CurrentFenceIndex;
	struct ID3D12Fence* m_Fence;
	HRESULT CreateFence();
	void DestroyFence();

	UINT m_m4xMSAAQuality;
	HRESULT DetermineMultisamplingDetails();
	void ClearMultisamplingDetails();

	struct ID3D12CommandQueue* m_CommandQueue;
	struct ID3D12CommandAllocator* m_CommandAllocator;
	struct ID3D12GraphicsCommandList* m_CommandList;
	HRESULT CreateCommandObjects();
	void DestroyCommandObjects();

	int m_CurrentBackbufferIndex;
	static const int m_SwapChainBufferCount;
	struct IDXGISwapChain1* m_SwapChain;
	HRESULT CreateSwapChain();
	void DestroySwapChain();

	struct ID3D12DescriptorHeap* m_RTVHeap;
	struct ID3D12DescriptorHeap* m_DSVHeap;
	HRESULT CreateDescriptorHeaps();
	void DestroyDescriptorHeaps();
	struct D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackbufferView() const;
	struct D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilBufferView() const;

	struct ID3D12Resource** m_SwapchainBuffers;
	HRESULT CreateRenderTargetViews();
	void DestroyRenderTargetViews();

	ID3D12Resource* m_DepthStencilBuffer;
	HRESULT CreateDepthStencilBuffer();
	void DestroyDepthStencilBuffer();

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;
	HRESULT SetupInitialViewportAndScissorRect();

	HRESULT FlushCommandQueue();

protected:

public:
	Renderer();
	~Renderer();

	const bool& IsInitialised();

	const DirectX::XMFLOAT4& GetClearColour() const;
	void SetClearColour(const DirectX::XMFLOAT4& newColour);

	static bool Initialise(Renderer& renderer, const HWND& windowHandle);
	static void Shutdown(Renderer& renderer);

	const int& GetWindowWidth() const;
	const int& GetWindowHeight() const;

	void ClearFrame();
	void PresentFrame();
};