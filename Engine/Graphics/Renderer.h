#pragma once

class Renderer
{
private:

	bool m_IsInitialised;
	HWND m_WindowHandle;

	enum DXGI_FORMAT m_BackbufferFormat;

	struct IDXGIFactory2* m_DXGIFactory;
	struct ID3D12Device* m_Device;
	HRESULT CreateDeviceAndFactory();
	void DestroyDeviceAndFactory();

	UINT m_RTVDescriptorHeapSize;
	UINT m_DSVDescriptorHeapSize;
	UINT m_CBVSRVDescriptorHeapSize;
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

protected:

public:
	Renderer();
	~Renderer();

	const bool& IsInitialised();

	static bool Initialise(Renderer& renderer, const HWND& windowHandle);
	static void Shutdown(Renderer& renderer);
};