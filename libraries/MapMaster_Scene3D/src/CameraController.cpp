# include "MapMaster/Scene3D/CameraController.hpp"

# include <iostream>
# include <raylib.h>
# include <rcamera.h>

void CameraController::setMoveSpeed (float moveSpeed) {
	m_moveSpeed = moveSpeed;
}

float CameraController::moveSpeed () const {
	return m_moveSpeed;
}

void CameraController::setRotationSpeed (float rotationSpeed) {
	m_rotationSpeed = rotationSpeed;
}

float CameraController::rotationSpeed () const {
	return m_rotationSpeed;
}

void CameraController::setCamera (Camera3D * camera) {
	m_camera = camera;
}

Camera3D * CameraController::camera () const {
	return m_camera;
}

void CameraController::updateCamera () const {
	float moveSpeedCoeff = 1.0F;
	float rotationSpeedCoeff = 1.0F;

	if (true == IsKeyDown (KEY_LEFT_SHIFT) || true == IsKeyDown (KEY_RIGHT_SHIFT)) {
		moveSpeedCoeff = 2;
		rotationSpeedCoeff = 2;
	}

	if (true == IsKeyDown (KEY_W)) { groundMoveForward (moveSpeedCoeff); }

	if (true == IsKeyDown (KEY_S)) { groundMoveBackward (moveSpeedCoeff); }

	if (true == IsKeyDown (KEY_A)) { groundMoveLeft (moveSpeedCoeff); }

	if (true == IsKeyDown (KEY_D)) { groundMoveRight (moveSpeedCoeff); }

	if (true == IsKeyDown (KEY_J)) { freeMoveBackward (moveSpeedCoeff); }

	if (true == IsKeyDown (KEY_K)) { freeMoveForward (moveSpeedCoeff); }

	if (true == IsKeyDown (KEY_Q)) { freeMoveUp (moveSpeedCoeff); }

	if (true == IsKeyDown (KEY_E)) { freeMoveDown (moveSpeedCoeff); }

	if (true == IsKeyDown (KEY_LEFT_CONTROL) || true == IsKeyDown (KEY_RIGHT_CONTROL)) {

		if (true == IsKeyDown (KEY_Z)) { freeRotateLeft (rotationSpeedCoeff); }

		if (true == IsKeyDown (KEY_X)) { freeRotateRight (rotationSpeedCoeff); }

		if (true == IsKeyDown (KEY_C)) { freeRotateDown (rotationSpeedCoeff); }

		if (true == IsKeyDown (KEY_V)) { freeRotateUp (rotationSpeedCoeff); }
	}
	else {

		if (true == IsKeyDown (KEY_Z)) { orbitalRotateLeft (rotationSpeedCoeff); }

		if (true == IsKeyDown (KEY_X)) { orbitalRotateRight (rotationSpeedCoeff); }

		if (true == IsKeyDown (KEY_C)) { orbitalRotateDown (rotationSpeedCoeff); }

		if (true == IsKeyDown (KEY_V)) { orbitalRotateUp (rotationSpeedCoeff); }
	}
}

void CameraController::groundMoveForward (float moveSpeedCoeff) const {
	 CameraMoveForward (m_camera, moveSpeedCoeff * m_moveSpeed * GetFrameTime (), true);
}

void CameraController::groundMoveBackward (float moveSpeedCoeff) const {
	 CameraMoveForward (m_camera, -moveSpeedCoeff * m_moveSpeed * GetFrameTime (), true);
}

void CameraController::groundMoveLeft (float moveSpeedCoeff) const {
	 CameraMoveRight (m_camera, -moveSpeedCoeff * m_moveSpeed * GetFrameTime (), true);
}

void CameraController::groundMoveRight (float moveSpeedCoeff) const {
	 CameraMoveRight (m_camera, moveSpeedCoeff * m_moveSpeed * GetFrameTime (), true);
}

void CameraController::freeMoveBackward (float moveSpeedCoeff) const {
	 CameraMoveForward (m_camera, -moveSpeedCoeff * m_moveSpeed * GetFrameTime (), false);
}

void CameraController::freeMoveForward (float moveSpeedCoeff) const {
	 CameraMoveForward (m_camera, moveSpeedCoeff * m_moveSpeed * GetFrameTime (), false);
}

void CameraController::freeMoveUp (float moveSpeedCoeff) const {
	 CameraMoveUp (m_camera, moveSpeedCoeff * m_moveSpeed * GetFrameTime ());
}

void CameraController::freeMoveDown (float moveSpeedCoeff) const {
	 CameraMoveUp (m_camera, -moveSpeedCoeff * m_moveSpeed * GetFrameTime ());
}

void CameraController::freeRotateLeft (float rotationSpeedCoeff) const {
	 CameraYaw (m_camera, rotationSpeedCoeff * m_rotationSpeed * GetFrameTime (), false);
}

void CameraController::freeRotateRight (float rotationSpeedCoeff) const {
	 CameraYaw (m_camera, -rotationSpeedCoeff * m_rotationSpeed * GetFrameTime (), false);
}

void CameraController::freeRotateDown (float rotationSpeedCoeff) const {
	 CameraPitch (m_camera, -rotationSpeedCoeff * m_rotationSpeed * GetFrameTime (), true, false, false);
}

void CameraController::freeRotateUp (float rotationSpeedCoeff) const {
	 CameraPitch (m_camera, rotationSpeedCoeff * m_rotationSpeed * GetFrameTime (), true, false, false);
}

void CameraController::orbitalRotateLeft (float rotationSpeedCoeff) const {
	 CameraYaw (m_camera, -rotationSpeedCoeff * m_rotationSpeed * GetFrameTime (), true);
}

void CameraController::orbitalRotateRight (float rotationSpeedCoeff) const {
	 CameraYaw (m_camera, rotationSpeedCoeff * m_rotationSpeed * GetFrameTime (), true);
}

void CameraController::orbitalRotateDown (float rotationSpeedCoeff) const {
	 CameraPitch (m_camera, rotationSpeedCoeff * m_rotationSpeed * GetFrameTime (), true, true, false);
}

void CameraController::orbitalRotateUp (float rotationSpeedCoeff) const {
	 CameraPitch (m_camera, -rotationSpeedCoeff * m_rotationSpeed * GetFrameTime (), true, true, false);
}
