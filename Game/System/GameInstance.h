#pragma once
#include <Graphics/Renderer.h>

class ConstantBuffer;
class LightBuffer;
class World;
class EventSystem;
class Model;

class GameInstance
{
private:
	bool m_IsInitalised;
	bool m_IsRunning;
	Renderer m_Renderer;
	EventSystem& m_EventSystem;
	ConstantBuffer* m_ConstantBuffer;
	LightBuffer* m_LightBuffer;
	World* m_World;
	Model* m_Flat2DSquare;
	Texture* m_TestTexture;
	Model* m_OverlapSquare;
	Texture* m_OverlapTexture;

public:
	GameInstance();
	~GameInstance();

	const bool& IsRunning();
	void SetIsRunning(const bool& state);

	bool Initialise(const HWND& windowHandle);
	void Shutdown();

	void OnResize(const int& w, const int& h);
	void OnKeyboardInput(const UINT& message, const WPARAM& wParam, const LPARAM& lParam);

	void ProcessEvents(const float& deltaTime);
	void Update(const float& deltaTime);
	void Render();
};

