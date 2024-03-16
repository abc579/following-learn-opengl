#pragma once

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <iostream>
#include <cmath>

enum class CameraMovementOptions {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

constexpr float Yaw{ -90.f };
constexpr float Pitch{ 0.0f };
constexpr float Speed{ 4.5f };
constexpr float Sensitivity{ 0.1f };
constexpr float Zoom{ 45.0f };

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL.
class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler Angles.
    float yaw;
    float pitch;

    // Camera Options.
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    // constructor with vectors
    Camera(glm::vec3 aPosition = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 aUp = glm::vec3(0.0f, 1.0f, 0.0f), float aYaw = Yaw, float aPitch = Pitch) {
        position = aPosition;
        worldUp = aUp;
        yaw = aYaw;
        pitch = aPitch;
        movementSpeed = Speed;
        mouseSensitivity = Sensitivity;
        zoom = Zoom;
        front = glm::vec3(0.0f, 0.0f, -1.0f);

        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 getMyOwnShittyViewMatrix(glm::vec3 aPosition, glm::vec3 aTarget, glm::vec3 aWorldUp) const {
        // We already have the position, what we need to compute is:
        // 1. X axis (direction)
        // 2. Z axis (right)
        // 3. Y axis (up)
        glm::vec3 zAxisMat = glm::normalize(aPosition - aTarget);
        glm::vec3 xAxisMat = glm::normalize(glm::cross(glm::normalize(aWorldUp), zAxisMat));
        glm::vec3 yAxisMat = glm::normalize(glm::cross(zAxisMat, xAxisMat));

        glm::mat4 rotationMat = glm::mat4(1.f);
        rotationMat[0][0] = xAxisMat.x;
        rotationMat[1][0] = xAxisMat.y;
        rotationMat[2][0] = xAxisMat.z;
        rotationMat[0][1] = yAxisMat.x;
        rotationMat[1][1] = yAxisMat.y;
        rotationMat[2][1] = yAxisMat.z;
        rotationMat[0][2] = zAxisMat.x;
        rotationMat[1][2] = zAxisMat.y;
        rotationMat[2][2] = zAxisMat.z;

        glm::mat4 translationMat = glm::mat4(1.f);
        translationMat[3][0] = -position.x;
        translationMat[3][1] = -position.y;
        translationMat[3][2] = -position.z;

        return rotationMat * translationMat;
    }

    void processKeyboard(const CameraMovementOptions direction, const float deltaTime) {
        const static float velocity = movementSpeed * deltaTime;

        if(direction == CameraMovementOptions::FORWARD) {
            position += front * velocity;
        } else if(direction == CameraMovementOptions::BACKWARD) {
            position -= front * velocity;
        } else if(direction == CameraMovementOptions::LEFT) {
            position -= right * velocity;
        } else if(direction == CameraMovementOptions::RIGHT) {
            position += right * velocity;
        } else if(direction == CameraMovementOptions::UP) {
            position += up * velocity;
        } else if(direction == CameraMovementOptions::DOWN) {
            position -= up * velocity;
        }

        // XXX: nasty way of fixing the player to the XZ plane.
        //front.y = 0.f;
    }

    void processMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch = true) {
        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        yaw += xOffset;
        pitch += yOffset;

        if(constrainPitch) {
            if(pitch > 89.0f) {
                pitch = 89.0f;
            } else if(pitch < -89.0f) {
                pitch = -89.0f;
            }
        }

        updateCameraVectors();
    }

    void processMouseScroll(float yOffset) {
        zoom -= yOffset;

        if(zoom < 1.0f) {
            zoom = 1.0f;
        } else if(zoom > 45.0f) {
            zoom = 45.0f;
        }
    }

private:
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        newFront.y = std::sin(glm::radians(pitch));
        newFront.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

        front = glm::normalize(newFront);

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));

        // XXX: another way of fixing the player to the XZ plane, doom style.
        // front.y = 0.f;
    }
};
