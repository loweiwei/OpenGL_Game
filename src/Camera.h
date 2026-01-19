#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front  = glm::vec3(0,0,-1);
    glm::vec3 Up;
    glm::vec3 RightVec;
    glm::vec3 WorldUp;

    float Yaw   = -90.0f;
    float Pitch =  15.0f;

    float Sensitivity = 0.15f;
    float Zoom = 45.0f;

    // --- TPS ---
    float Distance      = 12.0f;
    float MinDistance   = 5.0f;
    float MaxDistance   = 18.0f;
    glm::vec3 TargetOffset = glm::vec3(0, 2.0f, 0);

    Camera(glm::vec3 pos);

    glm::mat4 GetViewMatrix();
    void ProcessMouseMovement(float xoffset, float yoffset);
    void ProcessMouseScroll(float yoffset);

    // --- 正確的 Player 跟隨攝影機 ---
    void FollowPlayer(const glm::vec3& playerPos);

private:
    void updateCameraVectors();
};

#endif
