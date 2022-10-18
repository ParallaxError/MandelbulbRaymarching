//------------------------------
//- window.cpp
//------------------------------

// Includes
#include "window.h"
#include <wincodec.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#define ID_FILEMENUSAVE 1
#define ID_FILEMENUEXIT 2
#define ID_COLOURMENUCOLOUR1 3
#define ID_COLOURMENUCOLOUR2 4
#define ID_COLOURMENUANIMATED 5
#define ID_SETTINGSMENUQLTLO 6
#define ID_SETTINGSMENUQLTMD 7
#define ID_SETTINGSMENUQLTHI 8
#define ID_SETTINGSMENURESET 9

using namespace DirectX;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* window;
	HMENU hmenu;
	CHOOSECOLORW colourInfo;

	if (message == WM_NCCREATE)
	{
		// Find what instance this is being called from
		window = static_cast<Window*> (reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
	}
	else
	{
		// Already set pointer, retrieve window
		window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSEACTIVATE:
		// When you click to activate the window, we want Mouse to ignore that event
		return MA_ACTIVATEANDEAT;
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			window->m_renderer->ResizeSwapChain();
		}
		break;
	case WM_SIZING:
		window->m_renderer->ResizeSwapChain();
		break;
	case WM_COMMAND:
		hmenu = GetMenu(window->hwnd);

		// Menu
		switch (wParam)
		{
		// Save image
		case ID_FILEMENUSAVE:
		{
			OPENFILENAME sfn = { 0 };
			TCHAR szFile[300] = { 0 };

			// Fields for dialogue box
			sfn.lStructSize = sizeof(sfn);
			sfn.hwndOwner = hwnd;
			sfn.lpstrFile = szFile;
			sfn.nMaxFile = sizeof(szFile);
			sfn.lpstrFilter = L"PNG (*.png)\0*.png\0JPEG (*.jpg;*.jpeg;*.jpe;*.jfif)\0*.jpg;*.jpeg;*.jpe;*.jfif\0Bitmap (*.bmp)\0*.bmp\0";
			sfn.nFilterIndex = 1;
			sfn.lpstrFileTitle = NULL;
			sfn.nMaxFileTitle = 0;
			sfn.lpstrInitialDir = NULL;

			sfn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetSaveFileName(&sfn) == TRUE)
			{
				// Save texture to file
				GUID format;
				LPCWSTR extension = PathFindExtension(sfn.lpstrFile);

				if (wcscmp(extension, L".png") == 0)
				{
					format = GUID_ContainerFormatPng;
				}
				else if (wcscmp(extension, L".jpg") == 0 || wcscmp(extension, L".jpeg") == 0 || wcscmp(extension, L".jpe") == 0 || wcscmp(extension, L".jfif") == 0)
				{
					format = GUID_ContainerFormatJpeg;
				}
				else
				{
					format = GUID_ContainerFormatBmp;
				}

				window->m_renderer->SaveRenderToFile(sfn.lpstrFile, format);
				MessageBeep(MB_OK);
			}

			break;
		}
		// Quit
		case ID_FILEMENUEXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		// Colour selections
		case ID_COLOURMENUCOLOUR1:
			// Create colour dialog
			static COLORREF customColours1[16];// Custom Colours
			ZeroMemory(&colourInfo, sizeof(colourInfo));

			colourInfo.lStructSize = sizeof(colourInfo);
			colourInfo.hwndOwner = hwnd;
			colourInfo.lpCustColors = (LPDWORD)customColours1;
			colourInfo.Flags = CC_FULLOPEN;

			// Display dialog
			if (ChooseColor(&colourInfo))
			{
				window->m_renderer->colour1 = colourInfo.rgbResult;
			}

			break;
		case ID_COLOURMENUCOLOUR2:
			// Create colour dialog
			static COLORREF customColours2[16];// Custom Colours
			ZeroMemory(&colourInfo, sizeof(colourInfo));

			colourInfo.lStructSize = sizeof(colourInfo);
			colourInfo.hwndOwner = hwnd;
			colourInfo.lpCustColors = (LPDWORD)customColours2;
			colourInfo.Flags = CC_FULLOPEN;

			// Display dialog
			if (ChooseColor(&colourInfo))
			{
				window->m_renderer->colour2 = colourInfo.rgbResult;
			}

			break;
		case ID_COLOURMENUANIMATED:
			// First get HMenu status
			MENUITEMINFO menuInfo;
			ZeroMemory(&menuInfo, sizeof(menuInfo));
			menuInfo.cbSize = sizeof(menuInfo);
			menuInfo.fMask = MIIM_STATE;

			GetMenuItemInfo(hmenu, ID_COLOURMENUANIMATED, false, &menuInfo);

			// Now check if it is checked and toggle
			if (menuInfo.fState & MFS_CHECKED)
			{
				CheckMenuItem(hmenu, ID_COLOURMENUANIMATED, MF_UNCHECKED);
			}
			else
			{
				CheckMenuItem(hmenu, ID_COLOURMENUANIMATED, MF_CHECKED);
			}

			// Set shading on or off
			window->m_renderer->m_constants.animated = !(menuInfo.fState & MFS_CHECKED);

			break;
			// Quality settings
		case ID_SETTINGSMENUQLTLO:
			// Set checks
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTLO, MF_CHECKED);
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTMD, MF_UNCHECKED);
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTHI, MF_UNCHECKED);

			// Set quality
			window->m_renderer->m_constants.quality = 0;
			break;
		case ID_SETTINGSMENUQLTMD:
			// Set checks
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTLO, MF_UNCHECKED);
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTMD, MF_CHECKED);
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTHI, MF_UNCHECKED);

			// Set quality
			window->m_renderer->m_constants.quality = 1;
			break;

		case ID_SETTINGSMENUQLTHI:
			// Set checks
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTLO, MF_UNCHECKED);
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTMD, MF_UNCHECKED);
			CheckMenuItem(hmenu, ID_SETTINGSMENUQLTHI, MF_CHECKED);

			// Set quality
			window->m_renderer->m_constants.quality = 2;
			break;
		case ID_SETTINGSMENURESET:
			// Reset camera
			window->m_renderer->m_camera.m_position = { -1.3084f, 0.0610f, -2.8699f };
			window->m_renderer->m_camera.m_right = { 0.9063f, 0.0f, -0.4226f };
			window->m_renderer->m_camera.m_up = { 0.0221f, 0.9986f, 0.0474f };
			window->m_renderer->m_camera.m_look = { 0.422039f, -0.052336f, 0.905065f };
			break;
		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


// Create HWND
Window::Window(int width, int height, HINSTANCE hInstance)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); // Byte width
	wcex.style = CS_HREDRAW | CS_VREDRAW; // Style flags
	wcex.lpfnWndProc = WndProc; // Window Procedure
	wcex.cbClsExtra = 0; // Any extra bytes to be allocated
	wcex.cbWndExtra = 0; // Extra bytes for window instance
	wcex.hInstance = hInstance; // hInstance
	wcex.hIcon = nullptr; // Icon
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW); // Cursor
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Background colour
	wcex.lpszMenuName = nullptr; // Menu name
	wcex.lpszClassName = L"MainWindowClass"; // Class name
	wcex.hIconSm = nullptr; // Small icon

	// Register
	RegisterClassEx(&wcex);

	// Create window
	RECT rc = { 0, 0, width, height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	hwnd = CreateWindow(
		wcex.lpszClassName, // Class name
		L"Mandelbulb", // Window name
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX, // Style flags
		CW_USEDEFAULT, // X coordinate
		CW_USEDEFAULT, // Y coordinate
		rc.right - rc.left, // Width
		rc.bottom - rc.top, // Height
		nullptr, // Parent
		nullptr, // Menu
		hInstance, // hInstance
		this // lpParam
	);

	// Instantiate renderer
	m_renderer = new Renderer(hwnd);

	// Also create menu items
	SetupMenus(hwnd);
}

// Destructor
Window::~Window()
{

}

// Just a list of all the menus to create and handle
void Window::SetupMenus(HWND hwnd)
{
	// Create HMENU and populate
	HMENU hmenu = CreateMenu();

	// File menu
	HMENU hFileMenu = CreateMenu();
	AppendMenuW(hFileMenu, MF_STRING, ID_FILEMENUSAVE, L"Save Image");
	AppendMenuW(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(hFileMenu, MF_STRING, ID_FILEMENUEXIT, L"Exit Application");

	// Colour menu
	HMENU hColourMenu = CreateMenu();
	AppendMenuW(hColourMenu, MF_STRING, ID_COLOURMENUCOLOUR1, L"Set Colour 1");
	AppendMenuW(hColourMenu, MF_STRING, ID_COLOURMENUCOLOUR2, L"Set Colour 2");

	// Settings menu
	HMENU hSettingsMenu = CreateMenu();

	// Quality settings
	HMENU hQualityMenu = CreateMenu();
	AppendMenuW(hQualityMenu, MF_STRING | MF_CHECKED, ID_SETTINGSMENUQLTLO, L"Low");
	AppendMenuW(hQualityMenu, MF_STRING, ID_SETTINGSMENUQLTMD, L"Medium");
	AppendMenuW(hQualityMenu, MF_STRING, ID_SETTINGSMENUQLTHI, L"High");

	AppendMenuW(hSettingsMenu, MF_POPUP, (UINT_PTR)hQualityMenu, L"Quality");
	AppendMenuW(hSettingsMenu, MF_STRING | MF_UNCHECKED, ID_COLOURMENUANIMATED, L"Animation");
	AppendMenuW(hSettingsMenu, MF_STRING, ID_SETTINGSMENURESET, L"Reset Camera");

	// Main bar
	AppendMenuW(hmenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
	AppendMenuW(hmenu, MF_POPUP, (UINT_PTR)hColourMenu, L"Colour");
	AppendMenuW(hmenu, MF_POPUP, (UINT_PTR)hSettingsMenu, L"Settings");

	// Set menu
	SetMenu(hwnd, hmenu);
}
