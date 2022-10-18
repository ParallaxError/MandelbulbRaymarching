#pragma once

//------------------------------
//- renderer.h
//------------------------------

// Includes
#include "camera.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>
#include <exception>
#include <Mouse.h>

using namespace Microsoft::WRL;
using namespace DirectX;

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
};

_declspec(align(16)) struct SHADER_CONSTANTS_BUFFER
{
	// Projection view matrix
	DirectX::XMMATRIX projInverse;
	DirectX::XMMATRIX viewInverse;
	DirectX::XMFLOAT3 camPos;
	float padding1;

	// Screen dimensions
	int screenWidth;
	int screenHeight;

	// Animation and quality
	int animated;
	int quality;

	// Time since execution start
	float time;

	// Colours
	DirectX::XMFLOAT3 colour1;
	DirectX::XMFLOAT3 colour2;
	float padding2;
};

class Renderer
{
public:
	// Pointer to mouse singleton
	Mouse* mouse;
	// Constants
	SHADER_CONSTANTS_BUFFER m_constants;
	// Mandelbulb colours
	COLORREF colour1 = RGB(255, 255, 255);
	COLORREF colour2 = RGB(255, 255, 255);
	// Scene camera
	Camera m_camera;

	// Constructor
	Renderer(HWND hwnd);
	// Destructor
	~Renderer();

	// Render to RTV and present
	void Render();
	// Resize swapchain to render new window size
	void ResizeSwapChain();
	// Screenshot
	void SaveRenderToFile(LPCWSTR fileName, GUID format);
private:
	// Camera near and far dist
	float m_camNear = 0.1f;
	float m_camFar = 1000.0f;
	// Mouse tracker
	Mouse::ButtonStateTracker m_tracker;

	// Window hwnd
	HWND hwnd;

	// Device and context
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;

	// HWnd dimensions
	float m_width;
	float m_height;

	// Swapchain presents RTV result to the screen
	// https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nn-dxgi1_2-idxgiswapchain1
	ComPtr<IDXGISwapChain> m_swapChain;

	// IA related members
	ComPtr<ID3D11InputLayout> p_inputLayout;
	ComPtr<ID3D11Buffer> m_vertexBuffer; // Vertex buffer

	// Shader views
	ComPtr<ID3D11RenderTargetView> m_rtv; // Render target view for main image

	// Shaders
	ComPtr<ID3D11VertexShader> p_vertexShader;
	ComPtr<ID3D11PixelShader> p_pixelShader;

	ID3D11Buffer* p_constantBuffer = NULL;

	// General updates for a frame to frame basis
	void Update(float deltaTime);
	// Update constants
	HRESULT UpdateConstants();
};

// Helper functions for DirectX
inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch Win32 API errors.
		throw std::exception();
	}
}