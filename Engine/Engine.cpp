// Engine.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"

#include "Engine.h"
#include <windows.h>
#include <debugapi.h>

namespace A
{
	void fnEngine()
	{
		OutputDebugString(L"Hello World!");
		MessageBox(NULL, L"Test", L"Test", MB_OK);
	}
}
