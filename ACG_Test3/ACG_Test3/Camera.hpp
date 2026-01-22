#pragma once
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtc/matrix_transform.hpp"

// defining the directions the camera can move
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
    // camera position and direction vectors
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // rotation angles
    float Yaw;
    float Pitch;

    // camera settings
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor: setting up default values and calculating initial vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(10.0f), MouseSensitivity(0.1f), Zoom(45.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // calculating the view matrix so opengl knows how to render the scene based on where we are looking
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // moving the camera position based on keyboard input
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD) Position += Front * velocity;
        if (direction == BACKWARD) Position -= Front * velocity;
        if (direction == LEFT) Position -= Right * velocity;
        if (direction == RIGHT) Position += Right * velocity;
        // keep player on ground (optional: remove if you want to fly)
        // Position.y = 0.0f; 
    }

    // changing where the camera is looking based on mouse movement
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // stopping the camera from flipping over if we look too far up or down
        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        // we moved the mouse, so we need to recalculate the direction vectors
        updateCameraVectors();

    }

private:
    // doing the math to convert yaw and pitch angles into actual 3d vectors
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        // recalculating right and up vectors to keep everything straight relative to the world
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};