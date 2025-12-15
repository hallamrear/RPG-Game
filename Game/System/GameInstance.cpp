#include "pch.h"
#include "GameInstance.h"
#include <System/Debug.h>
#include <System/SceneLoader.h>
#include <System/GeometryLoader.h>
#include <Graphics/ConstantBuffer.h>
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
	m_IsInitalised &= SceneLoader::LoadSceneFromFileIntoWorld(m_Renderer, *m_World, "Resources/SceneLoaderTest.gltf");

	m_ConstantBuffer = new ConstantBuffer();
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->View, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->Projection, DirectX::XMMatrixIdentity());

	m_LightBuffer = new LightBuffer();

	int type = 0;
	for (size_t i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		m_LightBuffer->LightData[i].Enabled = 0;

		if (i % 4 == 0)
		{
			type++;
		}

		m_LightBuffer->LightData[i].Type = (Light::LIGHT_TYPE)type;
		m_LightBuffer->LightData[i].Position = DirectX::XMFLOAT4(0.0f, 5.0f, 0.0f, 1.0f);
		m_LightBuffer->LightData[i].Direction = DirectX::XMFLOAT4(-5.0f, -5.0f, 0.0f, 0.0f);
		m_LightBuffer->LightData[i].Ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_LightBuffer->LightData[i].Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_LightBuffer->LightData[i].Specular = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_LightBuffer->LightData[i].Attenuation = DirectX::XMFLOAT4(1.0f, 0.09f, 0.032f, 1.0f);
		m_LightBuffer->LightData[i].InnerCutoff = 0.91f;
		m_LightBuffer->LightData[i].OuterCutoff = 0.82f;
		m_LightBuffer->LightData[i].Strength = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	
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


	DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(sinf(timer) * 35.0f, 35.0f, cosf(timer) * 35.0f);
	DirectX::XMFLOAT3 zero = DirectX::XMFLOAT3(0.0f, 5.0f, 0.0f);
	DirectX::XMFLOAT3 dir = DirectX::XMFLOAT3(zero.x - pos.x, zero.y - pos.y, zero.z - pos.z);
	DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

	DirectX::XMFLOAT4 pos4 = DirectX::XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
	DirectX::XMFLOAT4 dir4 = DirectX::XMFLOAT4(dir.x, dir.y, dir.z, 0.0f);
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->View, DirectX::XMLoadFloat4x4(&m_Renderer.GetViewMatrix()));
	DirectX::XMStoreFloat4x4(&m_ConstantBuffer->Projection, DirectX::XMLoadFloat4x4(&m_Renderer.GetProjectionMatrix()));
	DirectX::XMStoreFloat4(&m_ConstantBuffer->CameraPosition, DirectX::XMLoadFloat4(&pos4));
	DirectX::XMStoreFloat4(&m_ConstantBuffer->CameraDirection, DirectX::XMLoadFloat4(&dir4));

	timer += deltaTime;
	t += deltaTime;

	DirectX::XMStoreFloat4x4(&m_Renderer.GetViewMatrix(), DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&pos),
			DirectX::XMLoadFloat3(&zero),
			DirectX::XMLoadFloat3(&up))));

	if (t > 1.0f)
	{
		int type = (int)m_LightBuffer->LightData[0].Type;
		type++;
		type = type % 3;

		Debug::LogMessage("LT: %i\n", type);

		m_LightBuffer->LightData[0].Enabled = 1;
		m_LightBuffer->LightData[0].Type = (Light::LIGHT_TYPE)type;
		m_LightBuffer->LightData[0].Position = pos4;
		m_LightBuffer->LightData[0].Direction = dir4;
		m_LightBuffer->LightData[0].Strength = DirectX::XMFLOAT4(10.0f, 10.0f, 10.0f, 10.0f);

		t = 0.0f;
	}

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
