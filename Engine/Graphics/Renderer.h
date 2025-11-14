#pragma once

class Renderer
{
private:
	bool m_IsInitialised;

	enum DXGI_FORMAT m_BackbufferFormat;

	struct IDXGIFactory* m_DXGIFactory;
	struct ID3D12Device* m_Device;
	HRESULT CreateDevice();
	void DestroyDevice();

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

protected:

public:
	Renderer();
	~Renderer();

	const bool& IsInitialised();

	static bool Initialise(Renderer& renderer);
	static void Shutdown(Renderer& renderer);
};