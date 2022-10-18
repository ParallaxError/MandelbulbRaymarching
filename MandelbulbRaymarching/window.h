#pragma once

//------------------------------
//- window.h
//------------------------------

// Includes
#include "renderer.h"

#include <Windows.h>
#include <Mouse.h>

// Window that displays render output and handles input
class Window
{
public:
	// HWND for this window
	HWND hwnd;
	// Renderer
	Renderer* m_renderer;

	// Constructor
	Window(int width, int height, HINSTANCE hInstance);

	// Destructor
	~Window();
private:
	// Called only inside constructor for modularity
	void SetupMenus(HWND hwnd);
};
