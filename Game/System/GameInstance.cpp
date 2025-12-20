#include "pch.h"
#include "GameInstance.h"
#include <System/Debug.h>
#include <System/SceneLoader.h>
#include <System/GeometryLoader.h>
#include <Graphics/ConstantBuffer.h>
#include <World/Entity.h>

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

	m_ConstantBuffer = new ConstantBuffer();
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->View, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->Projection, DirectX::XMMatrixIdentity());

	m_LightBuffer = new LightBuffer();

	for (size_t i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		m_LightBuffer->LightData[i].Enabled = 0;
		m_LightBuffer->LightData[i].Type = (Light::LIGHT_TYPE::DIRECTIONAL);
		m_LightBuffer->LightData[i].Position = DirectX::XMFLOAT4(0.0f, 5.0f, 0.0f, 1.0f);
		m_LightBuffer->LightData[i].Direction = DirectX::XMFLOAT4(0.0f, -5.0f, 0.0f, 0.0f);
		m_LightBuffer->LightData[i].Ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_LightBuffer->LightData[i].Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_LightBuffer->LightData[i].Specular = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_LightBuffer->LightData[i].Attenuation = DirectX::XMFLOAT4(1.0f, 0.09f, 0.032f, 1.0f);
		m_LightBuffer->LightData[i].InnerCutoff = 0.91f;
		m_LightBuffer->LightData[i].OuterCutoff = 0.82f;
		m_LightBuffer->LightData[i].Strength = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	m_LightBuffer->LightData[0].Enabled = 1;

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

	m_LightBuffer->LightData->Position = DirectX::XMFLOAT4(sinf(timer) * 550.0f, 50.0f, cosf(timer) * 550.0f, 1.0f);
	m_LightBuffer->LightData->Direction = DirectX::XMFLOAT4(
		m_LightBuffer->LightData->Position.x * -1.0f,
		m_LightBuffer->LightData->Position.y * -1.0f,
		m_LightBuffer->LightData->Position.z * -1.0f,
		0.0f);

	DirectX::XMStoreFloat4(&m_LightBuffer->LightData->Direction, DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&m_LightBuffer->LightData->Direction)));
	
	timer += deltaTime;
	t += deltaTime;

	Entity* e = m_World->GetEntity(5);
	DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 translation = { m_LightBuffer->LightData->Position.x, m_LightBuffer->LightData->Position.y, m_LightBuffer->LightData->Position.z };

	DirectX::XMStoreFloat4x4(&e->GetLocalMatrix(),
		DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
		DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z));

	DirectX::XMVECTOR cameraPosition = { 150.0f, 150.0f, 150.0f, 0.0f };
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


	if (t > 1.0f)
	{
		t = 0.0f;
	}

	if (m_World != nullptr)
	{
		m_World->Update(deltaTime);
	}

	for (size_t i = 0; i < MAX_LIGHT_COUNT; i++)
	{

		if(m_LightBuffer->LightData[i].Enabled)
		{
			Debug::LogMessage(
				"Light %i:\n" \
				"\t Pos: %f %f %f\n" \
				"\t Dir: %f %f %f\n",
				i,
				m_LightBuffer->LightData[i].Position.x, m_LightBuffer->LightData[i].Position.y, m_LightBuffer->LightData[i].Position.z,
				m_LightBuffer->LightData[i].Direction.x, m_LightBuffer->LightData[i].Direction.y, m_LightBuffer->LightData[i].Direction.z
			);
		}
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
