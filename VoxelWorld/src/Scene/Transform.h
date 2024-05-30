#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform
{
public:
    Transform()
        : position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f), scale(1.0f)
    {
    }

    // Get the transformation matrix
    glm::mat4 GetTransform() const
    {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rot = glm::mat4_cast(rotation);
        glm::mat4 scl = glm::scale(glm::mat4(1.0f), scale);
        return trans * rot * scl;
    }

    // Set and get position
    void SetPosition(const glm::vec3& pos) { position = pos; }
    glm::vec3 GetPosition() const { return position; }

    // Set and get rotation
    void SetRotation(const glm::quat& rot) { rotation = rot; }
    glm::quat GetRotation() const { return rotation; }

    // Set and get scale
    void SetScale(const glm::vec3& scl) { scale = scl; }
    glm::vec3 GetScale() const { return scale; }

private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};