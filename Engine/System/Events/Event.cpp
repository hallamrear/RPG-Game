#include "pch.h"
#include "Event.h"

Event::Event(const int& type)
{
	m_EventType = type;
	Data = EventData();
}

Event::~Event()
{
	m_EventType = UNDEFINED_EVENT;
	Data = EventData();
}

const int& Event::GetType() const
{
	return m_EventType;
}
