#pragma once
#include <Graphics/Renderer.h>

class ConstantBuffer;
class LightBuffer;
class World;

class GameInstance
{
private:
	bool m_IsInitalised;
	bool m_IsRunning;
	Renderer m_Renderer;
	ConstantBuffer* m_ConstantBuffer;
	LightBuffer* m_LightBuffer;
	World* m_World;

public:
	GameInstance();
	~GameInstance();

	const bool& IsRunning();
	void SetIsRunning(const bool& state);

	bool Initialise(const HWND& windowHandle);
	void Shutdown();

	void ProcessInput();
	void Update(const float& deltaTime);
	void Render();
};

