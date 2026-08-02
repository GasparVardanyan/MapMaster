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

	void groundMoveForward (float moveSpeedCoeff = 1.0F) const;
	void groundMoveBackward (float moveSpeedCoeff = 1.0F) const;
	void groundMoveLeft (float moveSpeedCoeff = 1.0F) const;
	void groundMoveRight (float moveSpeedCoeff = 1.0F) const;
	void freeMoveBackward (float moveSpeedCoeff = 1.0F) const;
	void freeMoveForward (float moveSpeedCoeff = 1.0F) const;
	void freeMoveUp (float moveSpeedCoeff = 1.0F) const;
	void freeMoveDown (float moveSpeedCoeff = 1.0F) const;
	void freeRotateLeft (float rotationSpeedCoeff = 1.0F) const;
	void freeRotateRight (float rotationSpeedCoeff = 1.0F) const;
	void freeRotateDown (float rotationSpeedCoeff = 1.0F) const;
	void freeRotateUp (float rotationSpeedCoeff = 1.0F) const;
	void orbitalRotateLeft (float rotationSpeedCoeff = 1.0F) const;
	void orbitalRotateRight (float rotationSpeedCoeff = 1.0F) const;
	void orbitalRotateDown (float rotationSpeedCoeff = 1.0F) const;
	void orbitalRotateUp (float rotationSpeedCoeff = 1.0F) const;

private:
	// NOLINTNEXTLINE(hicpp-uppercase-literal-suffix,cppcoreguidelines-avoid-magic-numbers)
	float m_moveSpeed = 5000.0F;
	// NOLINTNEXTLINE(hicpp-uppercase-literal-suffix,cppcoreguidelines-avoid-magic-numbers)
	float m_rotationSpeed = 0.75F;
	Camera3D * m_camera = nullptr;
};
