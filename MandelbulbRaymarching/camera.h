#pragma once

//------------------------------
//- camera.h
//------------------------------

// Includes
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
	// Coords and dir
	XMFLOAT3 m_position = { -1.3084f, 0.0610f, -2.8699f };
	XMFLOAT3 m_right = { 0.9063f, 0.0f, -0.4226f };
	XMFLOAT3 m_up = { 0.0221f, 0.9986f, 0.0474f };
	// Look dir
	XMFLOAT3 m_look = { 0.422039f, -0.052336f, 0.905065f };

	// Used for view frustum calculation
	float m_nearZ = 0.0f;
	float m_farZ = 0.0f;
	float m_aspect = 0.0f;
	float m_fovY = 0.0f;
	float m_nearWindowHeight = 0.0f;
	float m_farWindowHeight = 0.0f;

	// View and projection matrices
	XMFLOAT4X4 m_view = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	XMFLOAT4X4 m_proj = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// Constructor
	Camera();

	// Destructor
	~Camera();

	// Calculate frustum
	void SetLens(float fovY, float aspect, float zn, float zf);

	// Move camera
	void Strafe(float d, float deltaTime);
	void Walk(float d, float deltaTime);

	// Rotate camera
	void Pitch(float angle);
	void RotateY(float angle);

	// Rebuild view matrix
	void UpdateViewMatrix();
};
