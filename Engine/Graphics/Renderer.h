#pragma once

class Renderer
{
private:
	bool m_IsInitialised;

protected:

public:
	Renderer();
	~Renderer();

	const bool& IsInitialised();

	static bool Initialise(Renderer& renderer);
	static void Shutdown(Renderer& renderer);
};