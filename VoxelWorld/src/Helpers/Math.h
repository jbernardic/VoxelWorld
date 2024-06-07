#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Math {
    struct Transform
    {
        glm::vec3 translation = glm::vec3();
        glm::quat rotation = glm::quat();
        glm::vec3 scale = glm::vec3();

        Transform() = default;

        Transform(glm::mat4 transform)
        {
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transform, scale, rotation, translation, skew, perspective);
            this->scale = scale;
            this->translation = translation;
            this->rotation = rotation;
        }

        glm::mat4 ToMat4() const
        {
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), translation);
            glm::mat4 rot = glm::mat4_cast(rotation);
            glm::mat4 scl = glm::scale(glm::mat4(1.0f), scale);
            return trans * rot * scl;
        }
    };
}