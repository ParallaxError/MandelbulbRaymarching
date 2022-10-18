//------------------------------
//- main.cpp
//------------------------------

// Includes
#include "window.h"
#include "renderer.h"

#include <Windows.h>
#include <chrono>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) 
{
	// For calculating delta time
	float start, now;
	start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	// Declare main window
	Window window(1280, 720, hInstance);

	// Initialize mouse singleton
	std::unique_ptr<Mouse> mouse;
	mouse = std::make_unique<Mouse>();
	mouse->SetWindow(window.hwnd);

	window.m_renderer->mouse = &(*mouse); // Look into alternative to this
	ShowWindow(window.hwnd, nCmdShow);

	MSG msg;
	while (true)
	{
		// Updating delta time
		now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		window.m_renderer->m_constants.time = float(now - start);

		// Handle messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Render and present the scene
			window.m_renderer->Render();
		}

		if (msg.message == WM_QUIT)
			break;
	}

	return 0;
}