#pragma once

#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovementOptions {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

constexpr float Yaw{ -90.f };
constexpr float Pitch{ 0.0f };
constexpr float Speed{ 2.5f };
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
    Camera(glm::vec3 aPosition = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 aUp = glm::vec3(0.0f, 1.0f, 0.0f), float aYaw = Yaw, float aPitch = Pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(Speed), mouseSensitivity(Sensitivity), zoom(Zoom) {
        position = aPosition;
        worldUp = aUp;
        yaw = aYaw;
        pitch = aPitch;

        updateCameraVectors();
    }

    // constructor with scalar values
    // Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float aYaw, float aPitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(Speed), mouseSensitivity(Sensitivity), zoom(Zoom) {
    //     position = glm::vec3(posX, posY, posZ);
    //     worldUp = glm::vec3(upX, upY, upZ);
    //     yaw = aYaw;
    //     pitch = aPitch;
    //     updateCameraVectors();
    // }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
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
        }
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
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(newFront);

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};
