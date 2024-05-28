#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(float fov, float width, float height)
	:m_ViewMatrix(1.0f), m_ProjectionMatrix(1.0f)
{
	SetProjection(fov, width, height);
}

void Camera::SetProjection(float fov, float width, float height)
{
	m_ProjectionMatrix = glm::perspective(glm::radians(fov), width / height, 0.1f, 10000.0f);
}

void Camera::SetPosition(glm::vec3 position)
{
	m_Position = position;
	calculateMatrices();
}

void Camera::SetPitch(float pitch)
{
	m_Pitch = pitch;
	calculateMatrices();
}

void Camera::SetYaw(float yaw)
{
	m_Yaw = yaw;
	calculateMatrices();
}

glm::vec3 Camera::GetForwardVector()
{
	glm::vec3 direction;
	direction.x = -m_ViewMatrix[0][2];
	direction.y = -m_ViewMatrix[1][2];
	direction.z = -m_ViewMatrix[2][2];
	return direction;
}

glm::vec3 Camera::GetRightVector()
{
	glm::vec3 direction;
	direction.x = m_ViewMatrix[0][0];
	direction.y = m_ViewMatrix[1][0];
	direction.z = m_ViewMatrix[2][0];
	return direction;
}

glm::vec3 Camera::GetUpVector()
{
	glm::vec3 direction;
	direction.x = m_ViewMatrix[0][1];
	direction.y = m_ViewMatrix[1][1];
	direction.z = m_ViewMatrix[2][1];
	return direction;
}

void Camera::calculateMatrices()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	direction.y = sin(glm::radians(m_Pitch));
	direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

	glm::vec3 cameraPos = m_Position;
	glm::vec3 cameraFront = glm::normalize(direction);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	m_ViewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}