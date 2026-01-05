#include "pch.h"
#include "EventSystem.h"
#include <System/Debug.h>

EventSystem* EventSystem::m_Instance = nullptr;

class TestEvent : public Event
{
public:
	TestEvent() : Event(DEBUG_EVENT)
	{
		Data.GameEvent.QuestID = rand() % 10;
	}
};

EventSystem::EventSystem()
{
	m_Handlers = std::vector<EventHandler*>();
	m_PendingEvents = std::vector<Event*>();

	m_PendingEvents.push_back(new TestEvent());
}

EventSystem::~EventSystem()
{
	ClearPendingEvents();
}

void EventSystem::ProcessPendingEvents(const float& deltaTime)
{
	EventSystem& instance = GetInstance();

	const size_t handlerCount = instance.m_Handlers.size();
	const size_t pendingEventCount = instance.m_PendingEvents.size();

	for (size_t i = 0; i < handlerCount; i++)
	{
		for (size_t e = 0; e < pendingEventCount; e++)
		{
			instance.m_Handlers[i]->HandleEvent(deltaTime, *instance.m_PendingEvents[e]);
		}
	}

	ClearPendingEvents();
}

void EventSystem::ClearPendingEvents()
{
	EventSystem& instance = GetInstance();

	const size_t pendingEventCount = instance.m_PendingEvents.size();

	if (pendingEventCount < 1)
		return;

	for (size_t e = 0; e < pendingEventCount; e++)
	{
		if (instance.m_PendingEvents[e] != nullptr)
		{
			delete instance.m_PendingEvents[e];
			instance.m_PendingEvents[e] = nullptr;
		}
	}

	instance.m_PendingEvents.clear();
}

void EventSystem::AddHandler(EventHandler* eventHandler)
{
	if (eventHandler == nullptr)
		return;

	m_Handlers.push_back(eventHandler);
}

void EventSystem::RemoveHandler(EventHandler* eventHandler)
{
	auto itr = std::find(m_Handlers.begin(), m_Handlers.end(), eventHandler);
	CUSTOM_ASSERT((itr == m_Handlers.end()));
	m_Handlers.erase(itr);
}

bool EventSystem::PushEvent(Event* event)
{
	if (event == nullptr)
		return false;

	GetInstance().m_PendingEvents.push_back(event);

	return true;
}

EventSystem& EventSystem::GetInstance()
{
	if (m_Instance == nullptr)
	{
		m_Instance = new EventSystem();
	}

	return *m_Instance;
}

EventHandler::EventHandler()
{
	EventSystem::GetInstance().AddHandler(this);
}

EventHandler::~EventHandler()
{
	EventSystem::GetInstance().RemoveHandler(this);
}
