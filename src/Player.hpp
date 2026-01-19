#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "OBB.hpp"
#include "RobotModel.hpp"   // ✅ 新增：純視覺 robot（不影響碰撞）

class ObjObject;
class Shader;

class Player
{
public:
    glm::vec3 Position;
    glm::vec3 Velocity = glm::vec3(0);

    float Yaw;
    float Speed = 25.0f;

    bool isInAir = false;
    float verticalVelocity = 0.0f;
    float gravity = -35.0f;
    float jumpSpeed = 12.0f;
    bool isOnPlatform = false;

    unsigned int VAO = 0;
    unsigned int VBO = 0;

    OBB obbLocal;
    OBB obb;

    RobotModel robot;       // ✅ 純畫圖用，不參與 OBB

    Player(glm::vec3 pos)
        : Position(pos), Yaw(-90.0f)
    {
        initOBB();
    }

    void InitModel();
    void Draw(Shader& shader, float t);

    void applyMovement(float dt, const std::vector<ObjObject*>& objs);

    void initOBB();
    void updateOBB();
};
