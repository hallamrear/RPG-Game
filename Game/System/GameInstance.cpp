#include "pch.h"
#include "GameInstance.h"
#include <System/Debug.h>
#include <Graphics/Geometry/Model.h>
#include <System/GeometryLoader.h>
#include <Graphics/ConstantBuffer.h>

Model model;

GameInstance::GameInstance()
{
	m_IsInitalised = false;
	m_IsRunning = false;
	m_ConstantBuffer = nullptr;
}

GameInstance::~GameInstance()
{
	CUSTOM_ASSERT(m_IsInitalised == false);
}

const bool& GameInstance::IsRunning()
{
	return m_IsRunning;
}

void GameInstance::SetIsRunning(const bool& state)
{
	m_IsRunning = state;
}

bool GameInstance::Initialise(const HWND& windowHandle)
{
	if(m_IsInitalised)
	{ 
		Debug::LogWarning("Calling initialise on an existing instance.\n");
		return false;
	}

	if (windowHandle == NULL)
	{
		Debug::LogWarning("Passing an invalid window handle.\n");
		return false;
	}

	m_IsInitalised = true;

	m_IsInitalised &= Renderer::Initialise(m_Renderer, windowHandle);

	GeometryLoader::Load(m_Renderer, model, "Resources/Test_Model.gltf");

	m_ConstantBuffer = new ConstantBuffer();
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->World, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->View, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH({ 0.0f, 1.0f, -5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f })));
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->Projection, DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(70.0f, 1920.0f / 1080.0f, 1.0f, 1000.0f)));

	//Setting to closed as the first reference to the command list will open it.
	if (m_Renderer.GetCommandList())
	{
		m_Renderer.GetCommandList()->Close();
	}

	SetIsRunning(m_IsInitalised);
	return m_IsInitalised;
}

void GameInstance::Shutdown()
{
	if (!m_IsInitalised)
		return;

	if (m_ConstantBuffer)
	{
		delete m_ConstantBuffer;
		m_ConstantBuffer = nullptr;
	}

	Renderer::Shutdown(m_Renderer);

	m_IsInitalised = false;
}

void GameInstance::ProcessInput()
{
	if (!IsRunning())
		return;
}

static float timer = 0.0f;
static float t = 0.0f;
static int index = 0;

ConstantBuffer cb[MAX_NUM_ENTITIES];
DirectX::XMFLOAT3 modelPos;

void GameInstance::Update(const float& deltaTime)
{
	if (!IsRunning())
		return;

	const float radius = 35.0f;
	DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(sinf(timer) * radius, 10.0f, cosf(timer) * radius);
	DirectX::XMFLOAT3 zero = DirectX::XMFLOAT3(0.0f, pos.y, 0.0f);
	modelPos = zero;
	DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

	timer += deltaTime;
	t += deltaTime;

	if (t > 0.33f)
	{
		index++;
		t = 0.0f;
	}

	DirectX::XMStoreFloat4x4(&m_Renderer.GetViewMatrix(), DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&pos),
			DirectX::XMLoadFloat3(&zero),
			DirectX::XMLoadFloat3(&up))));

	for (int i = 0; i < 6; i++)
	{
		DirectX::XMStoreFloat4x4(&cb[i].World, DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationRollPitchYaw(0.0f, timer * 5.0f, 0.0f) * DirectX::XMMatrixTranslation(-5.0f + (2.5f * i), 0.0f, 0.0f)));
		DirectX::XMStoreFloat4x4(&cb[i].View, DirectX::XMLoadFloat4x4(&m_Renderer.GetViewMatrix()));
		DirectX::XMStoreFloat4x4(&cb[i].Projection, DirectX::XMLoadFloat4x4(&m_Renderer.GetProjectionMatrix()));
	}

	m_Renderer.SetClearColour(DirectX::XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f));
}

void GameInstance::Render()
{
	if (!IsRunning())
		return;

	m_Renderer.ClearFrame();

	for (size_t i = 0; i < 6; i++)
	{
		m_Renderer.UpdateConstantBuffer(cb[i], i);
		model.TestRender(index + (i * 1), m_Renderer);
	}

	m_Renderer.PresentFrame();
}
