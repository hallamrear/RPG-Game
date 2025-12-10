#pragma once
#include <Graphics/DX12Includes.h>
#include <Defines.h>

class ConstantBuffer;

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
	struct ID3D12DescriptorHeap* m_SRVHeap;
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

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_DefaultInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_ColourOnlyInputLayout;
	HRESULT CreateInputLayout();
	void DestroyInputLayout();

	char** m_ConstantBufferAddressArray;
	ID3D12Resource** m_ConstantBufferGPUUploaderArray;
	HRESULT CreateConstantBuffers();
	void DestroyConstantBuffers();

	ID3D12DescriptorHeap** m_CBVHeaps;
	HRESULT CreateConstantBufferHeap();
	void DestroyConstantBufferHeap();

	ID3D12RootSignature* m_RootSignature;
	HRESULT CreateRootSignatureAndDescriptorTable();
	void DestroyRootSignatureAndDescriptorTable();


	ID3DBlob* m_DefaultPixelShaderBlob;
	ID3DBlob* m_DefaultVertexShaderBlob;
	ID3DBlob* m_ColourOnlyPixelShaderBlob;
	ID3DBlob* m_ColourOnlyVertexShaderBlob;
	HRESULT FindAndCreateShaders();
	HRESULT ReadShaderData(const std::string& filename, ID3DBlob*& targetBlob);
	void DestroyLoadedShaders();

	ID3D12PipelineState* m_ColourOnlyPipeline;
	ID3D12PipelineState* m_DefaultPipeline;
	HRESULT CreateGraphicsPipelines();
	void DestroyGraphicsPipelines();

	DirectX::XMFLOAT4X4 m_ViewMatrix;
	DirectX::XMFLOAT4X4 m_ProjectionMatrix;

protected:

public:
	Renderer();
	~Renderer();

	const bool& IsInitialised() const;

	ID3D12CommandQueue* GetCommandQueue();
	const ID3D12CommandQueue* GetCommandQueue() const;

	HRESULT ResetCommandList();
	ID3D12GraphicsCommandList* GetCommandList();
	const ID3D12GraphicsCommandList* GetCommandList() const;
	HRESULT ExecuteAndResetCommandList();

	const ID3D12Device* GetDevice() const;
	ID3D12Device* GetDevice();

	struct D3D12_CPU_DESCRIPTOR_HANDLE GetSRVDescriptorHeapStart() const;

	HRESULT CreateDefaultBuffer(ID3D12Resource*& defaultBuffer, ID3D12Resource*& gpuUploadBuffer, const void* data, const size_t& sizeBytes);

	const DirectX::XMFLOAT4& GetClearColour() const;
	void SetClearColour(const DirectX::XMFLOAT4& newColour);

	HRESULT UpdateConstantBuffer(ConstantBuffer& cb, const int& index);

	static bool Initialise(Renderer& renderer, const HWND& windowHandle);
	static void Shutdown(Renderer& renderer);

	const int& GetWindowWidth() const;
	const int& GetWindowHeight() const;

	const DirectX::XMFLOAT4X4& GetProjectionMatrix() const;
	DirectX::XMFLOAT4X4& GetProjectionMatrix();
	const DirectX::XMFLOAT4X4& GetViewMatrix() const;
	DirectX::XMFLOAT4X4& GetViewMatrix();

	void ClearFrame();
	HRESULT FlushCommandQueue();
	void PresentFrame();
};