#include "pch.h"
#include "GameInstance.h"
#include <System/Debug.h>
#include <System/SceneLoader.h>
#include <System/GeometryLoader.h>
#include <System/TextureLoader.h>
#include <Graphics/BufferStructures.h>
#include <Graphics/Texturing/Texture.h>
#include <Graphics/Texturing/Material.h>
#include <World/Entity.h>
#include <Graphics/Geometry/Model.h>
#include <World/World.h>

GameInstance::GameInstance()
{
	m_IsInitalised = false;
	m_IsRunning = false;
	m_ConstantBuffer = nullptr;
	m_LightBuffer = nullptr;
	m_World = nullptr;
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

	m_World = new World();
	m_IsInitalised &= SceneLoader::LoadSceneFromFileIntoWorld(m_Renderer, *m_World, "Resources/Map/Map.gltf");
	m_IsInitalised &= SceneLoader::LoadSceneFromFileIntoWorld(m_Renderer, *m_World, "Resources/Suzanne.gltf");

	m_ConstantBuffer = new ConstantBuffer();
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->View, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->Projection, DirectX::XMMatrixIdentity());

	m_LightBuffer = new LightBuffer();

	for (size_t i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		float x = (float)((int)i % 4) - 2.0f;
		float y = (float)((int)i / 4) - 2.0f;

		m_LightBuffer->LightData[i].Enabled = 1;
		m_LightBuffer->LightData[i].Type = (Light::LIGHT_TYPE::POINT);
		m_LightBuffer->LightData[i].Position = DirectX::XMFLOAT4(x * 150.0f, 5.0f, y * 150.0f, 1.0f);
		DirectX::XMFLOAT3 dir = { 0.0f, -1.0f, 0.0f };
		DirectX::XMStoreFloat4(&m_LightBuffer->LightData[i].Direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir)));

		float r = max((float)(rand() % 255) / 255.0f, 0.5f);
		float g = max((float)(rand() % 255) / 255.0f, 0.5f);
		float b = max((float)(rand() % 255) / 255.0f, 0.5f);

		m_LightBuffer->LightData[i].Diffuse = DirectX::XMFLOAT4(r, g, b, 1.0f);
		m_LightBuffer->LightData[i].Specular = DirectX::XMFLOAT3(r, g, b);
		m_LightBuffer->LightData[i].SpecularPower = 32.0f;
		//m_LightBuffer->LightData[i].Attenuation = DirectX::XMFLOAT4(1.0f, 0.09f, 0.032f, 1.0f);
		m_LightBuffer->LightData[i].Attenuation = DirectX::XMFLOAT4(1.0f, 0.007f, 0.0002f, 1.0f);
		m_LightBuffer->LightData[i].InnerCutoff = 0.91f;
		m_LightBuffer->LightData[i].OuterCutoff = 0.82f;
		m_LightBuffer->LightData[i].Strength = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	m_LightBuffer->LightData[0].Type = (Light::LIGHT_TYPE::DIRECTIONAL);
	m_LightBuffer->LightData[0].Direction = { 0.0f, -1.0f, 0.0f, 0.0f };
	m_LightBuffer->LightData[0].Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_LightBuffer->LightData[0].Specular = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);

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

	if (m_ConstantBuffer != nullptr)
	{
		delete m_ConstantBuffer;
		m_ConstantBuffer = nullptr;
	}

	if (m_LightBuffer != nullptr)
	{
		delete m_LightBuffer;
		m_LightBuffer = nullptr;
	}

	Renderer::Shutdown(m_Renderer);

	m_IsInitalised = false;
}

void GameInstance::OnResize(const int& w, const int& h)
{
	if (m_IsInitalised == false)
	{
		Debug::LogWarning("Resize method called on instance while not initialised.\n");
		return;
	}

	HRESULT hr = m_Renderer.ResizeSwapchain(w, h);

	if (FAILED(hr))
	{
		Debug::LogWarning("Failed to resize swapchain for game instance.\n");
	}
}

void GameInstance::ProcessInput()
{
	if (!IsRunning())
		return;
}

static float timer = 0.0f;
static float t = 0.0f;
static int index = 0;

void GameInstance::Update(const float& deltaTime)
{
	if (!IsRunning())
		return;

	DirectX::XMVECTOR up = { 0.0f, 1.0f, 0.0f };
	
	timer += deltaTime;
	t += deltaTime;

	DirectX::XMFLOAT3 dir = { sinf(timer), cosf(timer), 0.0f };
	DirectX::XMStoreFloat4(&m_LightBuffer->LightData[0].Direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir)));
	if (t > 2.5f)
	{
		t = 0.0f;
	}

	DirectX::XMVECTOR cameraPosition = { sinf(timer) * 800.0f, 200.0f, cosf(timer) * 800.0f };
	DirectX::XMVECTOR cameraTarget = { 0.0f, 0.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR cameraDirection = DirectX::XMVectorSubtract(cameraTarget, cameraPosition);

	DirectX::XMStoreFloat4x4(&m_Renderer.GetViewMatrix(), DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(
			cameraPosition,
			cameraTarget,
			up)));

	DirectX::XMStoreFloat4(&m_ConstantBuffer->CameraPosition, cameraPosition);
	DirectX::XMStoreFloat4(&m_ConstantBuffer->CameraDirection, cameraDirection);
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->View, DirectX::XMLoadFloat4x4(&m_Renderer.GetViewMatrix()));
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->Projection, DirectX::XMLoadFloat4x4(&m_Renderer.GetProjectionMatrix()));

	if (m_World != nullptr)
	{
		m_World->Update(deltaTime);
	}
}

void GameInstance::Render()
{
	if (!IsRunning())
		return;

	m_Renderer.ClearFrame();

	if (m_LightBuffer != nullptr)
	{
		m_Renderer.UpdateLightingBuffer(*m_LightBuffer);
	}

	if (m_ConstantBuffer != nullptr)
	{
		m_Renderer.UpdateConstantBuffer(*m_ConstantBuffer);
	}
		
	if (m_World != nullptr)
	{
		m_World->Render(m_Renderer);
	}

	m_Renderer.PresentFrame();
}
