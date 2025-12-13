#pragma once
#include <Graphics/Renderer.h>

class ConstantBuffer;
class LightBuffer;
class MaterialBuffer;

class GameInstance
{
private:
	bool m_IsInitalised;
	bool m_IsRunning;
	Renderer m_Renderer;
	ConstantBuffer* m_ConstantBuffers;
	LightBuffer* m_LightBuffer;
	MaterialBuffer* m_MaterialBuffer;

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

