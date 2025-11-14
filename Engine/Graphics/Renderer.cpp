#include "pch.h"
#include "Renderer.h"
#include <System/Debug.h>

Renderer::Renderer()
{
    m_IsInitialised = false;
}

Renderer::~Renderer()
{
    CUSTOM_ASSERT(IsInitialised() == false);
}

const bool& Renderer::IsInitialised()
{
    return m_IsInitialised;
}

bool Renderer::Initialise(Renderer& renderer)
{
    if (renderer.IsInitialised())
    {
        Debug::LogWarning("Calling initialise on a renderer object that already exists.\n");
        return false;
    }



    if (renderer.IsInitialised() == false)
    {
        Debug::LogSevere("Failed to initialise renderer.\n");
    }

    return renderer.m_IsInitialised;
}

void Renderer::Shutdown(Renderer& renderer)
{
    if (!renderer.IsInitialised())
    {
        Debug::LogWarning("Calling shutdown on a renderer object that doesn't exist.");
        return;
    }
}
