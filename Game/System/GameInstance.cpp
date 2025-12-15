#include "pch.h"
#include "GameInstance.h"
#include <System/Debug.h>
#include <Graphics/Geometry/Model.h>
#include <System/SceneLoader.h>
#include <System/GeometryLoader.h>
#include <Graphics/ConstantBuffer.h>
#include <World/World.h>
#include <World/Entity.h>

#define TEST_MODEL_DRAWGAP 20.0f
#define TEST_MODEL_DRAWS_X 10
#define TEST_MODEL_DRAWS_Y 10
#define TEST_MODEL_DRAWS (TEST_MODEL_DRAWS_X * TEST_MODEL_DRAWS_Y)

Model model;
Model map;

Entity eMap;
Entity eModel;

GameInstance::GameInstance()
{
	m_IsInitalised = false;
	m_IsRunning = false;
	m_ConstantBuffers = nullptr;
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
	//m_IsInitalised &= SceneLoader::LoadSceneFromFileIntoWorld(m_Renderer, *m_World, "Resources/SceneLoaderTest.gltf");

	GeometryLoader::Load(m_Renderer, map, "Resources/Map/Map.gltf");

	m_ConstantBuffers = new ConstantBuffer[MAX_NUM_ENTITIES];

	for (size_t i = 0; i < MAX_NUM_ENTITIES; i++)
	{
		DirectX::XMStoreFloat4x4(&m_ConstantBuffers[i].View, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&m_ConstantBuffers[i].Projection, DirectX::XMMatrixIdentity());
	}

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

	if (m_ConstantBuffers != nullptr)
	{
		delete[] m_ConstantBuffers;
		m_ConstantBuffers = nullptr;
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

	timer += deltaTime;
	t += deltaTime;

	DirectX::XMStoreFloat4x4(&m_Renderer.GetViewMatrix(), DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&pos),
			DirectX::XMLoadFloat3(&zero),
			DirectX::XMLoadFloat3(&up))));

	const DirectX::XMFLOAT3 startPos = DirectX::XMFLOAT3(((TEST_MODEL_DRAWS_X * TEST_MODEL_DRAWGAP * -1) / 2.0f), 0.0f, ((TEST_MODEL_DRAWS_Y * TEST_MODEL_DRAWGAP * -1) / 2.0f));
	
	for (size_t x = 0; x < TEST_MODEL_DRAWS_X; x++)
	{
		for (size_t y = 0; y < TEST_MODEL_DRAWS_Y; y++)
		{
			int i = (y * TEST_MODEL_DRAWS_X) + x;

			if (i > MAX_NUM_ENTITIES)
				break;			

			DirectX::XMStoreFloat4x4(&m_ConstantBuffers[i].View, DirectX::XMLoadFloat4x4(&m_Renderer.GetViewMatrix()));
			DirectX::XMStoreFloat4x4(&m_ConstantBuffers[i].Projection, DirectX::XMLoadFloat4x4(&m_Renderer.GetProjectionMatrix()));

			DirectX::XMStoreFloat4(&m_ConstantBuffers[i].CameraPosition, DirectX::XMLoadFloat4(&pos4));
			DirectX::XMStoreFloat4(&m_ConstantBuffers[i].CameraDirection, DirectX::XMLoadFloat4(&dir4));
		}
	}

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

	m_Renderer.UpdateLightingBuffer(*m_LightBuffer);

	m_Renderer.UpdateConstantBuffer(m_ConstantBuffers[0], 0);
	
	DirectX::XMFLOAT4X4 identity;
	DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
	//DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(sinf(timer * 25.0f), 0.0f, cosf(timer) * 25.0f)));
	eMap.SetLocalMatrix(identity);

	for (size_t i = 1; i < TEST_MODEL_DRAWS; i++)
	{
		if (i > MAX_NUM_ENTITIES)
			break;

		//DirectX::XMFLOAT4X4 matrix;
		//DirectX::XMStoreFloat4x4(&m_ConstantBuffers[i].World, DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(0.05f, 0.05f, 0.05f) * DirectX::XMMatrixTranslation(startPos.x + (TEST_MODEL_DRAWGAP * x), 0.0f, startPos.z + (TEST_MODEL_DRAWGAP * y))));
		//
		//m_Renderer.UpdateConstantBuffer(m_ConstantBuffers[i], i);
		//model.TestRender(m_Renderer);
	}
	
	if (m_World != nullptr)
	{
		eMap.Render(m_Renderer, map);
		m_World->Render(m_Renderer, model);
	}


	m_Renderer.PresentFrame();
}
