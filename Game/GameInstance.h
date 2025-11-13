#pragma once

class GameInstance
{
private:
	bool m_IsInitalised;
	bool m_IsRunning;

public:
	GameInstance();
	~GameInstance();

	const bool& IsRunning();
	void SetIsRunning(const bool& state);

	bool Initialise();
	void Shutdown();

	void ProcessInput();
	void Update(const float& deltaTime);
	void Render();
};

