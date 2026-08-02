# pragma once

extern "C" {
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct Camera3D Camera3D;
}

class CameraController {
public:
	void setMoveSpeed (float moveSpeed);
	void setRotationSpeed (float rotationSpeed);
	void setCamera (Camera3D * camera);

	[[nodiscard]] float moveSpeed () const;
	[[nodiscard]] float rotationSpeed () const;
	[[nodiscard]] Camera3D * camera () const;

	void updateCamera () const;

private:
	// NOLINTNEXTLINE(hicpp-uppercase-literal-suffix,cppcoreguidelines-avoid-magic-numbers)
	float m_moveSpeed = 5000.0F;
	// NOLINTNEXTLINE(hicpp-uppercase-literal-suffix,cppcoreguidelines-avoid-magic-numbers)
	float m_rotationSpeed = 0.75F;
	Camera3D * m_camera = nullptr;
};
