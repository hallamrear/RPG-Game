#pragma once
#include <vector>
#include <System/Events/Event.h>

class EventHandler
{
protected:
	EventHandler();

public:
	virtual ~EventHandler() = 0;
	virtual bool HandleEvent(const float& deltaTime, const Event& event) = 0;
};

class EventSystem
{
private:
	friend class EventHandler;
	static EventSystem* m_Instance;
	std::vector<EventHandler*> m_Handlers;
	std::vector<Event*> m_PendingEvents;

protected:
	EventSystem();

	static void ClearPendingEvents();
	void AddHandler(EventHandler* eventHandler);
	void RemoveHandler(EventHandler* eventHandler);

public:
	~EventSystem();

	static bool PushEvent(Event* event);
	static EventSystem& GetInstance();
	static void ProcessPendingEvents(const float& deltaTime);
};