#include "pch.h"
#include "GameInstance.h"
#include <System/Debug.h>

GameInstance::GameInstance()
{
	m_IsInitalised = false;
	m_IsRunning = false;
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

bool GameInstance::Initialise()
{
	if(m_IsInitalised)
	{ 
		Debug::LogWarning("Calling initialise on an existing instance.\n");
		return false;
	}

	m_IsInitalised = true;

	m_IsInitalised &= Renderer::Initialise(m_Renderer);

	SetIsRunning(m_IsInitalised);
	return m_IsInitalised;
}

void GameInstance::Shutdown()
{
	if (!m_IsInitalised)
		return;

	Renderer::Shutdown(m_Renderer);

	m_IsInitalised = false;
}

void GameInstance::ProcessInput()
{
	if (!IsRunning())
		return;

	Debug::LogMessage("Process Input\n");
}

void GameInstance::Update(const float& deltaTime)
{
	if (!IsRunning())
		return;

	Debug::LogMessage("Update\n");
}

void GameInstance::Render()
{
	if (!IsRunning())
		return;

	Debug::LogMessage("Render\n");
}
