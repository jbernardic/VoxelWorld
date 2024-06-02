#pragma once
#include <glm/glm.hpp>
class Camera
{
public:
	Camera(float fov_deg, float width, float height);
	Camera() = default;
	void SetProjection(float fov_deg, float width, float height);
	void SetPosition(glm::vec3 position);
	void SetPitch(float pitch_deg);
	void SetYaw(float yaw_deg);
	glm::vec3 GetForwardVector();
	glm::vec3 GetRightVector();
	glm::vec3 GetUpVector();
	const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	const glm::mat4& GetViewProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix;; }
	const glm::vec3& GetPosition() const { return m_Position; }
	float GetPitch() const { return m_Pitch; }
	float GetYaw() const { return m_Yaw; }
private:
	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
	float m_Pitch = 0.0f;
	float m_Yaw = 270.0f;
	void calculateMatrices();
};