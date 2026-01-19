#include "Player.hpp"
#include "Shader.h"
#include <glad/glad.h>
#include <cmath>
#include "Collision.hpp"
#include "WorldManager.hpp"
#include <iostream>
// ✅ RobotModel 會用到
unsigned int gCubeVAO = 0;

void Player::InitModel()
{
    // 你原本的 vertices/VAO/VBO 建立（保持）
    float vertices[] = {
        // positions         normals
        -0.5f, 0.0f, 0.5f,  0,0,1,
         0.5f, 0.0f, 0.5f,  0,0,1,
         0.5f, 1.0f, 0.5f,  0,0,1,
         0.5f, 1.0f, 0.5f,  0,0,1,
        -0.5f, 1.0f, 0.5f,  0,0,1,
        -0.5f, 0.0f, 0.5f,  0,0,1,

        -0.5f, 0.0f,-0.5f,  0,0,-1,
         0.5f, 0.0f,-0.5f,  0,0,-1,
         0.5f, 1.0f,-0.5f,  0,0,-1,
         0.5f, 1.0f,-0.5f,  0,0,-1,
        -0.5f, 1.0f,-0.5f,  0,0,-1,
        -0.5f, 0.0f,-0.5f,  0,0,-1,

        -0.5f, 0.0f,-0.5f, -1,0,0,
        -0.5f, 0.0f, 0.5f, -1,0,0,
        -0.5f, 1.0f, 0.5f, -1,0,0,
        -0.5f, 1.0f, 0.5f, -1,0,0,
        -0.5f, 1.0f,-0.5f, -1,0,0,
        -0.5f, 0.0f,-0.5f, -1,0,0,

         0.5f, 0.0f,-0.5f, 1,0,0,
         0.5f, 0.0f, 0.5f, 1,0,0,
         0.5f, 1.0f, 0.5f, 1,0,0,
         0.5f, 1.0f, 0.5f, 1,0,0,
         0.5f, 1.0f,-0.5f, 1,0,0,
         0.5f, 0.0f,-0.5f, 1,0,0,

        -0.5f, 1.0f,-0.5f, 0,1,0,
         0.5f, 1.0f,-0.5f, 0,1,0,
         0.5f, 1.0f, 0.5f, 0,1,0,
         0.5f, 1.0f, 0.5f, 0,1,0,
        -0.5f, 1.0f, 0.5f, 0,1,0,
        -0.5f, 1.0f,-0.5f, 0,1,0,

        -0.5f, 0.0f,-0.5f, 0,-1,0,
         0.5f, 0.0f,-0.5f, 0,-1,0,
         0.5f, 0.0f, 0.5f, 0,-1,0,
         0.5f, 0.0f, 0.5f, 0,-1,0,
        -0.5f, 0.0f, 0.5f, 0,-1,0,
        -0.5f, 0.0f,-0.5f, 0,-1,0,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // ✅ robot 用同一顆 cube VAO 畫所有部位
    gCubeVAO = VAO;
}

void Player::Draw(Shader& shader, float t)
{
    // base transform：玩家位置 + 朝向（你原本的碰撞Yaw不變）
    glm::mat4 base(1.0f);
    const float visualLift = 0.8f;  // 先試 0.7，太高就改 0.5，太低就改 0.9
    base = glm::translate(base, Position + glm::vec3(0.0f, visualLift, 0.0f));
    //base = glm::translate(base, Position);
    float modelYawOffset = 360.0f;
    base = glm::rotate(base, glm::radians(Yaw + modelYawOffset), glm::vec3(0,1,0));

    //base = glm::rotate(base, glm::radians(Yaw), glm::vec3(0,1,0));
    // ✅ 真正的「正臉方向」：把模型 local +Z 轉到 world
    glm::vec3 faceWorld = glm::normalize(glm::vec3(base * glm::vec4(0,0,1,0)));
    // 也可以一起印右方( local +X )
    glm::vec3 rightWorld = glm::normalize(glm::vec3(base * glm::vec4(1,0,0,0)));

    /*static float lastYaw = 99999.f;
    if (fabs(Yaw - lastYaw) > 1.0f) {
        std::cout << "[Draw] Yaw=" << Yaw
                  << " FaceWorld=(" << faceWorld.x << "," << faceWorld.y << "," << faceWorld.z << ")"
                  << " RightWorld=(" << rightWorld.x << "," << rightWorld.y << "," << rightWorld.z << ")\n";
        lastYaw = Yaw;
    }*/
    robot.draw(shader, base, t);
}


void Player::initOBB()
{
    obbLocal.center   = glm::vec3(0, 0.7f, 0);
    obbLocal.halfSize = glm::vec3(0.65f, 0.7f, 0.65f);
    obbLocal.orientation = glm::mat3(1.0f);

    obb = obbLocal;  // 初始化 world obb
}


void Player::updateOBB()
{
    float rad = glm::radians(Yaw);
    float c = cos(rad), s = sin(rad);

    // column-major basis vectors
    glm::mat3 R(
        glm::vec3( c, 0, -s ),
        glm::vec3( 0, 1,  0 ),
        glm::vec3( s, 0,  c )
    );

    obb.orientation = R;

    // center = 腳底 + 一半高度
    obb.center = Position + glm::vec3(0, obb.halfSize.y, 0);
    obb.halfSize = obbLocal.halfSize;
}



void Player::applyMovement(float dt, const std::vector<ObjObject*>& objs)
{
    const float COLLISION_EPS = 0.03f;

    // ===============================
    // 1. Gravity
    // ===============================
    verticalVelocity += gravity * dt;
    Velocity.y = verticalVelocity;

    glm::vec3 move = Velocity * dt;

    // Player yaw rotation
    float rad = glm::radians(Yaw);
    float c = cos(rad), s = sin(rad);

    glm::mat3 R(
        glm::vec3(c,0,-s),
        glm::vec3(0,1,0),
        glm::vec3(s,0,c)
    );

    OBB future = obbLocal;
    future.orientation = R;


    // ===============================
    // 2. X Axis (horizontal wall collision)
    // ===============================
    glm::vec3 tryPos = Position + glm::vec3(move.x, 0, 0);
    future.center = tryPos + R * obbLocal.center;

    for (auto obj : objs)
    {
        glm::vec3 push;
        if (OBBvsOBB(future, obj->obb, push))
        {
            // Ignore vertical separation (Y 交給下一段處理)
            if (fabs(push.y) > fabs(push.x)) continue;

            Position.x += push.x;
            Velocity.x = 0;
            goto Y_AXIS;
        }
    }
    Position.x = tryPos.x;


Y_AXIS:
    tryPos = Position + glm::vec3(0, move.y, 0);
    future.center = tryPos + R * obbLocal.center;

    bool landed = false;
    isOnPlatform = false;

    for (auto obj : objs)
    {
        // -------------------------------------------------
        // Step 1：判斷玩家 XZ 是否在平台的投影範圍中
        // -------------------------------------------------
        glm::vec3 local =
            glm::transpose(obj->obb.orientation) *
            (future.center - obj->obb.center);

        glm::vec3 h = obj->obb.halfSize;

        bool insideXZ =
            fabs(local.x) <= h.x + 0.01f &&
            fabs(local.z) <= h.z + 0.01f;

        if (!insideXZ) continue;


        // -------------------------------------------------
        // Step 2：找出平台真正的上方向（UP normal）
        // -------------------------------------------------
        glm::vec3 worldUp(0,1,0);

        glm::vec3 axisX = obj->obb.orientation[0];
        glm::vec3 axisY = obj->obb.orientation[1];
        glm::vec3 axisZ = obj->obb.orientation[2];

        float dx = fabs(glm::dot(axisX, worldUp));
        float dy = fabs(glm::dot(axisY, worldUp));
        float dz = fabs(glm::dot(axisZ, worldUp));

        glm::vec3 up =
            (dx >= dy && dx >= dz) ? axisX :
            (dy >= dx && dy >= dz) ? axisY :
                                    axisZ;

        if (glm::dot(up, worldUp) < 0)
            up = -up; // flip，使其朝上


        // -------------------------------------------------
        // Step 3：平台沿 UP 軸方向的半高
        // -------------------------------------------------
        float halfH =
            fabs(glm::dot(up, axisX)) * h.x +
            fabs(glm::dot(up, axisY)) * h.y +
            fabs(glm::dot(up, axisZ)) * h.z;

        float platformTop    =  halfH;
        float platformBottom = -halfH;


        // -------------------------------------------------
        // Step 4：玩家沿 UP 軸方向的位置
        // -------------------------------------------------
        glm::vec3 diff = future.center - obj->obb.center;

        float projY = glm::dot(diff, up);

        float playerHalf   = future.halfSize.y;
        float playerBottom = projY - playerHalf;
        float playerTop    = projY + playerHalf;


        // =================================================
        // ⭐⭐ 下落落地判斷（從上方落到平台頂）
        // =================================================
        bool movingDown    = move.y < 0;
        bool playerAbove   = projY > platformBottom;                 // 不從下穿上
        bool touchingTop   = fabs(playerBottom - platformTop) <= 0.05f;

        if (movingDown && playerAbove && touchingTop)
        {
            float corr = platformTop - playerBottom;
            Position += up * corr;

            verticalVelocity = 0;
            isInAir = false;
            isOnPlatform = true;
            landed = true;
            break;
        }


        // =================================================
        // ⭐⭐ 往上跳 → 撞到平台底部
        // =================================================
        bool movingUp       = move.y > 0;
        bool playerBelow    = projY < platformTop;                   // 確實在平台下方
        bool touchingBottom = fabs(playerTop - platformBottom) <= 0.05f;

        if (movingUp && playerBelow && touchingBottom)
        {
            float corr = platformBottom - playerTop;  // 往下推
            Position += up * corr;

            verticalVelocity = 0; // 停止向上速度
            // 不設定 isOnPlatform（因為是天花板）
            break;
        }
    }

    // ========== 更新 Y ==========
    if (!landed)
    {
        Position.y = tryPos.y;
        isInAir = true;
    }
Z_AXIS:
    // ===============================
    // 4. Z Axis horizontal collision
    // ===============================
    tryPos = Position + glm::vec3(0, 0, move.z);
    future.center = tryPos + R * obbLocal.center;

    for (auto obj : objs)
    {
        glm::vec3 push;
        if (OBBvsOBB(future, obj->obb, push))
        {
            if (fabs(push.y) > fabs(push.z)) continue;

            Position.z += push.z;
            Velocity.z = 0;
            goto FINISH;
        }
    }
    Position.z = tryPos.z;


FINISH:
    updateOBB();
    Velocity.x = 0;
    Velocity.z = 0;
}

