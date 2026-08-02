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
    // Camera speeds based on frame time
    float cameraMoveSpeed = m_moveSpeed*GetFrameTime();
    float cameraRotationSpeed = m_rotationSpeed*GetFrameTime();
    // float cameraPanSpeed = CAMERA_PAN_SPEED*GetFrameTime();
    // float cameraOrbitalSpeed = CAMERA_ORBITAL_SPEED*GetFrameTime();

	// NOLINTBEGIN(readability-braces-around-statements,hicpp-braces-around-statements)
	if (IsKeyDown (KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) cameraMoveSpeed *= 2, cameraRotationSpeed *= 2;

	if (IsKeyDown(KEY_W)) CameraMoveForward(m_camera, cameraMoveSpeed, true);
	if (IsKeyDown(KEY_A)) CameraMoveRight(m_camera, -cameraMoveSpeed, true);
	if (IsKeyDown(KEY_S)) CameraMoveForward(m_camera, -cameraMoveSpeed, true);
	if (IsKeyDown(KEY_D)) CameraMoveRight(m_camera, cameraMoveSpeed, true);
	if (IsKeyDown(KEY_J)) CameraMoveForward(m_camera, -cameraMoveSpeed, false);
	if (IsKeyDown(KEY_K)) CameraMoveForward(m_camera, cameraMoveSpeed, false);
	if (IsKeyDown(KEY_Q)) CameraMoveUp(m_camera, cameraMoveSpeed);
	if (IsKeyDown(KEY_E)) CameraMoveUp(m_camera, -cameraMoveSpeed);

	bool ctrlDown = true == IsKeyDown (KEY_LEFT_CONTROL) || true == IsKeyDown (KEY_RIGHT_CONTROL);

	bool rotateAroundTarget = false == ctrlDown;
	float x = 1;
	if (true == ctrlDown) {
		x = -1;
	}

	if (IsKeyDown(KEY_Z)) CameraYaw(m_camera, x * -cameraRotationSpeed, rotateAroundTarget);
	if (IsKeyDown(KEY_X)) CameraYaw(m_camera, x * cameraRotationSpeed, rotateAroundTarget);
	if (IsKeyDown(KEY_C)) CameraPitch(m_camera, x * cameraRotationSpeed, true, rotateAroundTarget, false);
	if (IsKeyDown(KEY_V)) CameraPitch(m_camera, x * -cameraRotationSpeed, true, rotateAroundTarget, false);
	// NOLINTEND(readability-braces-around-statements,hicpp-braces-around-statements)
}
