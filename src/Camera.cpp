#include "Camera.h"

Camera::Camera(glm::vec3 pos)
    : Position(pos), WorldUp(0,1,0)
{
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset)
{
    xoffset *= Sensitivity;
    yoffset *= Sensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (Pitch >  45.0f) Pitch = 45.0f;
    if (Pitch < -20.0f) Pitch = -20.0f;

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Distance -= yoffset * 0.8f;
    Distance = glm::clamp(Distance, MinDistance, MaxDistance);
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    RightVec = glm::normalize(glm::cross(Front, WorldUp));
    Up       = glm::normalize(glm::cross(RightVec, Front));
}

void Camera::FollowPlayer(const glm::vec3& playerPos)
{
    float yawRad   = glm::radians(Yaw);       // ⭐ 用鏡頭的 Yaw
    float pitchRad = glm::radians(Pitch);

    float horiz = Distance * cos(pitchRad);
    float vert  = Distance * sin(pitchRad);

    float offsetX = horiz * cos(yawRad);
    float offsetZ = horiz * sin(yawRad);

    Position.x = playerPos.x - offsetX;
    Position.z = playerPos.z - offsetZ;
    Position.y = playerPos.y + vert + TargetOffset.y;

    Front = glm::normalize((playerPos + TargetOffset) - Position);
    RightVec = glm::normalize(glm::cross(Front, WorldUp));
    Up       = glm::normalize(glm::cross(RightVec, Front));
}

