#pragma once

#define UNDEFINED_EVENT 0x00000000
#define DEBUG_EVENT 0x00000001
#define INPUT_EVENT 0x00000002
#define CAMERA_EVENT 0x00000003

struct InputData
{
	int Key = -1;
};

struct WindowEvent
{
	int Width = -1;
	int Height = -1;
};

struct GameEvent
{
	int QuestID = -1;
};

union EventData
{
	InputData Input;
	WindowEvent Window;
	GameEvent GameEvent;

	inline EventData() {};
	inline ~EventData() {};
};

class Event
{
private:
	int m_EventType;

protected:

public:
	Event(const int& type);
	~Event();

	const int& GetType() const;
	EventData Data;
};
