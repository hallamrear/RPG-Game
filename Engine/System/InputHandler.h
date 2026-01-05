#pragma once
#include <System/Events/EventSystem.h>
#include <vector>

class InputHandler : public EventHandler
{
public:
	enum INPUT_STATE : int
	{
		UP = 2,
		HELD = 1,
		DOWN = 0
	};

	enum INPUT_TYPE : int
	{
		MOUSE,
		KEYBOARD
	};

	struct InputEvent
	{
		INPUT_TYPE Type;
		INPUT_STATE NewState;
	};

private:

protected:
	static void PushInputEvent(const InputEvent& event);
	static void Update(const float& deltaTime);

public:
	InputHandler();
	~InputHandler();
};


