// Includes
#include "renderer.h"

#include <d3dcompiler.h>
#include <ScreenGrab.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

// Constructor
Renderer::Renderer(HWND hwnd)
{
	// Initializing members
	{
		// Get height and width
		this->hwnd = hwnd;
		RECT rect;
		m_width = 800;
		m_height = 600;
		if (GetWindowRect(hwnd, &rect))
		{
			m_width = rect.right - rect.left;
			m_height = rect.bottom - rect.top;
		}

		// Allocate space for constants
		ZeroMemory(&m_constants, sizeof(m_constants));
	}

	// Device, swapchain and context creation
	{
		// Create Device
		D3D_FEATURE_LEVEL lvl[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
										  D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

		UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_create_device_flag
		// "Creates a device that supports the debug layer." https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-layers
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		// Swap chain desc
		DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hwnd;
		swapChainDesc.Windowed = true;

		// Flip sequential
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		m_device = nullptr;
		m_context = nullptr;

		ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			createDeviceFlags,
			NULL,
			0,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			m_swapChain.ReleaseAndGetAddressOf(),
			m_device.ReleaseAndGetAddressOf(),
			lvl,
			m_context.ReleaseAndGetAddressOf())
		);
	}

	// RTV creation
	{
		ComPtr<ID3D11Texture2D> framebuffer;
		ThrowIfFailed(m_swapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			(void**)&framebuffer));

		ThrowIfFailed(m_device->CreateRenderTargetView(
			framebuffer.Get(), 0, m_rtv.ReleaseAndGetAddressOf()));
	}

	// Shader compilation and creation, and defining vertex buffer + layout
	{
		ComPtr<ID3DBlob> p_vsBlob;
		ComPtr<ID3DBlob> p_psBlob;
		ComPtr<ID3DBlob> p_errorBlob;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		// Compiling vertex shader
		HRESULT hr = D3DCompileFromFile(L"main.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0",
			flags, 0, p_vsBlob.ReleaseAndGetAddressOf(), p_errorBlob.ReleaseAndGetAddressOf());

		// Compiling pixel shader
		hr = D3DCompileFromFile(L"main.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0",
			flags, 0, p_psBlob.ReleaseAndGetAddressOf(), p_errorBlob.ReleaseAndGetAddressOf());

		// Creating shaders
		ThrowIfFailed(m_device->CreateVertexShader(p_vsBlob->GetBufferPointer(), p_vsBlob->GetBufferSize(), NULL, p_vertexShader.ReleaseAndGetAddressOf()));
		ThrowIfFailed(m_device->CreatePixelShader(p_psBlob->GetBufferPointer(), p_psBlob->GetBufferSize(), NULL, p_pixelShader.ReleaseAndGetAddressOf()));

		D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Vertex position
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // UV
		};

		ThrowIfFailed(m_device->CreateInputLayout(inputElementDescs, _countof(inputElementDescs), p_vsBlob->GetBufferPointer(),
			p_vsBlob->GetBufferSize(), p_inputLayout.ReleaseAndGetAddressOf()));
	}

	// Creating vertex and index buffer 
	{
		// Initial vertex data
		Vertex vertices[6] = {
			{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) }
		};

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = sizeof(vertices);
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA srData = { 0 };
		srData.pSysMem = vertices;

		ThrowIfFailed(m_device->CreateBuffer(&vertexBufferDesc, &srData, m_vertexBuffer.ReleaseAndGetAddressOf()));
	}
}

// Destructor
Renderer::~Renderer()
{
	// Nothing to do upon destruction
}

void Renderer::Render()
{
	// First constants buffer
	Update(1.0f / 60.0f);

	const float clearColour[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	// Clear RTV
	m_context->ClearRenderTargetView(m_rtv.Get(), clearColour);

	// Update width and height
	RECT rect;
	if (GetWindowRect(hwnd, &rect))
	{
		m_width = rect.right - rect.left;
		m_height = rect.bottom - rect.top;
	}

	// Create viewport
	D3D11_VIEWPORT viewport = {
		0.0f,
		0.0f,
		(float)(m_width),
		(float)(m_height),
		0.0f,
		1.0f
	};
	m_context->RSSetViewports(1, &viewport);

	// Set RTV
	ID3D11RenderTargetView* p_rtv = m_rtv.Get();
	m_context->OMSetRenderTargets(1, &p_rtv, NULL);

	// Input assembler
	UINT vertexStride = sizeof(Vertex);
	UINT vertexOffset = 0;

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(p_inputLayout.Get());
	ID3D11Buffer* p_vb = m_vertexBuffer.Get();
	m_context->IASetVertexBuffers(0, 1, &p_vb, &vertexStride, &vertexOffset);

	// Set shaders
	m_context->VSSetShader(p_vertexShader.Get(), NULL, 0);
	m_context->PSSetShader(p_pixelShader.Get(), NULL, 0);

	// Draw
	m_context->Draw(6, 0);

	// And finally present!
	m_swapChain->Present(1, 0);
}

// Updates for frame to frame basis
void Renderer::Update(float deltaTime)
{
	// Set camera lens to account for aspect changes
	float aspect = m_width / m_height;
	m_camera.SetLens(0.78539816339f /*pi/4*/, aspect, m_camNear, m_camFar);

	float step = 10.0f;

	// For any movement, update sample as well
	// W
	if (GetAsyncKeyState(0x57) & 0x8000)
	{
		m_camera.Walk(step, deltaTime);
	}
	// S
	if (GetAsyncKeyState(0x53) & 0x8000)
	{
		m_camera.Walk(-step, deltaTime);
	}
	// A
	if (GetAsyncKeyState(0x41) & 0x8000)
	{
		m_camera.Strafe(-step, deltaTime);
	}
	// D
	if (GetAsyncKeyState(0x44) & 0x8000)
	{
		m_camera.Strafe(step, deltaTime);
	}

	// Camera
	auto state = mouse->GetState();
	m_tracker.Update(state);
	mouse->SetMode(state.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);

	if (state.positionMode == Mouse::MODE_RELATIVE)
	{
		float dx = XMConvertToRadians(1.0f * static_cast<float>(state.x));
		float dy = XMConvertToRadians(1.0f * static_cast<float>(state.y));

		m_camera.Pitch(dy);
		m_camera.RotateY(dx);
	}

	m_camera.UpdateViewMatrix();

	// Set shader-side matrix
	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = XMLoadFloat4x4(&m_camera.m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_camera.m_proj);

	m_constants.projInverse = DirectX::XMMatrixInverse(nullptr, proj);
	m_constants.viewInverse = DirectX::XMMatrixInverse(nullptr, view);
	m_constants.camPos = m_camera.m_position;

	// Also update screen size
	m_constants.screenHeight = m_height;
	m_constants.screenWidth = m_width;

	// Update constants
	ThrowIfFailed(UpdateConstants());
}

// Update constants buffer
HRESULT Renderer::UpdateConstants()
{
	// Set colour float3s to colour COLOURREFS
	m_constants.colour1 = XMFLOAT3(GetRValue(colour1), GetGValue(colour1), GetBValue(colour1));
	m_constants.colour2 = XMFLOAT3(GetRValue(colour2), GetGValue(colour2), GetBValue(colour2));

	// Fill in a buffer description
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(SHADER_CONSTANTS_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));

	InitData.pSysMem = &m_constants;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	// Create the buffer.
	HRESULT hr = m_device->CreateBuffer(&cbDesc, &InitData,
		&p_constantBuffer);

	if (FAILED(hr))
		return hr;

	// Set the buffer.
	m_context->VSSetConstantBuffers(0, 1, &p_constantBuffer);
	m_context->PSSetConstantBuffers(0, 1, &p_constantBuffer);

	p_constantBuffer->Release();

	return hr;
}

void Renderer::ResizeSwapChain()
{
	// Get height and width
	RECT rect;
	if (GetWindowRect(hwnd, &rect))
	{
		m_width = rect.right - rect.left;
		m_height = rect.bottom - rect.top;
	}

	// Release RTV
	m_rtv.Reset();

	// Resize swap chain
	m_swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	// Get back buffer and resize viewport
	ComPtr<ID3D11Texture2D> framebuffer;
	ThrowIfFailed(m_swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&framebuffer));

	ThrowIfFailed(m_device->CreateRenderTargetView(
		framebuffer.Get(), 0, m_rtv.ReleaseAndGetAddressOf()));

	D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
	framebuffer->GetDesc(&backBufferDesc);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(backBufferDesc.Width);
	viewport.Height = static_cast<float>(backBufferDesc.Height);
	viewport.MinDepth = D3D11_MIN_DEPTH;
	viewport.MaxDepth = D3D11_MAX_DEPTH;

	m_context->RSSetViewports(1, &viewport);
}

// Screenshot
void Renderer::SaveRenderToFile(LPCWSTR fileName, GUID format)
{
	// Get back buffer
	ComPtr<ID3D11Texture2D> framebuffer;
	ThrowIfFailed(m_swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&framebuffer));

	DirectX::SaveWICTextureToFile(m_context.Get(), framebuffer.Get(), format, fileName);
}