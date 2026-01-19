#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Player.hpp"
#include "ObjObject.hpp"
#include <iostream>
#include <glad/glad.h>
// ======================================================
// 取得 OBB 的 3 個世界軸
// ======================================================
inline void GetOBBAxes(const OBB& box, glm::vec3 axes[3])
{
    axes[0] = glm::normalize(box.orientation[0]);
    axes[1] = glm::normalize(box.orientation[1]);
    axes[2] = glm::normalize(box.orientation[2]);
}

// ======================================================
// SAT 投影
// ======================================================
inline void ProjectOBB(const OBB& obb, const glm::vec3& axis, float& minP, float& maxP)
{
    glm::vec3 axes[3];
    GetOBBAxes(obb, axes);

    float centerProj = glm::dot(axis, obb.center);
    float radius =
        fabs(glm::dot(axis, axes[0]) * obb.halfSize.x) +
        fabs(glm::dot(axis, axes[1]) * obb.halfSize.y) +
        fabs(glm::dot(axis, axes[2]) * obb.halfSize.z);

    minP = centerProj - radius;
    maxP = centerProj + radius;
}

// ======================================================
// SAT — OBB vs OBB
// ======================================================
inline bool OBBvsOBB(const OBB& A, const OBB& B, glm::vec3& pushOut)
{
    glm::vec3 axesA[3], axesB[3];
    GetOBBAxes(A, axesA);
    GetOBBAxes(B, axesB);

    glm::vec3 testAxes[15];
    int idx = 0;

    // A 的 3 軸
    for (int i = 0; i < 3; i++)
        testAxes[idx++] = axesA[i];

    // B 的 3 軸
    for (int i = 0; i < 3; i++)
        testAxes[idx++] = axesB[i];

    // Cross Axes
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            glm::vec3 c = glm::cross(axesA[i], axesB[j]);
            if (glm::length(c) > 1e-4f)
                testAxes[idx++] = glm::normalize(c);
        }

    float bestOverlap = 1e9f;
    glm::vec3 bestAxis(0);

    for (int i = 0; i < idx; i++)
    {
        glm::vec3 axis = glm::normalize(testAxes[i]);

        float minA, maxA, minB, maxB;
        ProjectOBB(A, axis, minA, maxA);
        ProjectOBB(B, axis, minB, maxB);

        float overlap = std::min(maxA, maxB) - std::max(minA, minB);

        if (overlap <= 0)
            return false;

        if (overlap < bestOverlap)
        {
            bestOverlap = overlap;
            bestAxis = axis;
        }
    }

    // direction correction
    glm::vec3 dir = A.center - B.center;
    if (glm::dot(dir, bestAxis) < 0)
        bestAxis = -bestAxis;

    pushOut = bestAxis * bestOverlap;
    return true;
}

// ======================================================
// Player vs 所有 ObjObject 最終呼叫
// ======================================================
inline void ResolvePlayerOBBCollision(Player& player, const std::vector<ObjObject*>& objs)
{
    bool collided = true;
    int safety = 0;   // 防止無限迴圈

    while (collided && safety < 5)
    {
        collided = false;

        for (auto obj : objs)
        {
            glm::vec3 pushOut;

            if (OBBvsOBB(player.obb, obj->obb, pushOut))
            {
                // ---- 只移動水平 ----
                pushOut.y = 0.0f;

                // ---- 推出來（小幅多次）----
                player.Position -= pushOut * 0.6f;

                player.updateOBB();
                collided = true;
            }
        }

        safety++;
    }
}
inline void GetOBBCorners(const OBB& obb, glm::vec3 out[8])
{
    glm::vec3 X = obb.orientation[0] * obb.halfSize.x;
    glm::vec3 Y = obb.orientation[1] * obb.halfSize.y;
    glm::vec3 Z = obb.orientation[2] * obb.halfSize.z;

    glm::vec3 c = obb.center;

    out[0] = c + X + Y + Z;
    out[1] = c + X + Y - Z;
    out[2] = c + X - Y + Z;
    out[3] = c + X - Y - Z;
    out[4] = c - X + Y + Z;
    out[5] = c - X + Y - Z;
    out[6] = c - X - Y + Z;
    out[7] = c - X - Y - Z;
}

inline bool PointInTriangleXZ(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec2 p2(p.x, p.z);
    glm::vec2 a2(a.x, a.z);
    glm::vec2 b2(b.x, b.z);
    glm::vec2 c2(c.x, c.z);

    glm::vec2 v0 = c2 - a2;
    glm::vec2 v1 = b2 - a2;
    glm::vec2 v2 = p2 - a2;

    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0 && v >= 0 && (u + v) <= 1);
}
inline float TriangleHeightAtXZ(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));

    // 解 plane：n.x*(x - a.x) + n.y*(y - a.y) + n.z*(z - a.z) = 0
    float y = a.y - (n.x * (p.x - a.x) + n.z * (p.z - a.z)) / n.y;

    return y;
}
inline void FindOBBTopFace_ByUp(const OBB& obb, glm::vec3 corners[8], int outTop[4])
{
    glm::vec3 worldUp(0,1,0);

    glm::vec3 axisX = glm::normalize(obb.orientation[0]);
    glm::vec3 axisY = glm::normalize(obb.orientation[1]);
    glm::vec3 axisZ = glm::normalize(obb.orientation[2]);

    float dotX = fabs(glm::dot(axisX, worldUp));
    float dotY = fabs(glm::dot(axisY, worldUp));
    float dotZ = fabs(glm::dot(axisZ, worldUp));

    glm::vec3 upAxis;

    if (dotX >= dotY && dotX >= dotZ)
        upAxis = axisX;
    else if (dotY >= dotX && dotY >= dotZ)
        upAxis = axisY;
    else
        upAxis = axisZ;

    if (glm::dot(upAxis, worldUp) < 0)
        upAxis = -upAxis;

    float proj[8];
    float maxP = -1e9;

    for (int i = 0; i < 8; i++)
    {
        proj[i] = glm::dot(corners[i], upAxis);
        if (proj[i] > maxP) maxP = proj[i];
    }

    int count = 0;
    for (int i = 0; i < 8; i++)
    {
        if (fabs(proj[i] - maxP) < 0.01f)
            outTop[count++] = i;

        if (count == 4) break;
    }
}

inline void FindOBBTopFace(const OBB& obb, glm::vec3 corners[8], int outTop[4])
{
    glm::vec3 worldUp(0, 1, 0);

    float bestDot = -1.0f;
    int bestAxis = 1;

    for (int i = 0; i < 3; i++)
    {
        float d = glm::dot(obb.orientation[i], worldUp);
        if (fabs(d) > fabs(bestDot))
        {
            bestDot = d;
            bestAxis = i;
        }
    }

    // 若 dot < 0，要反向
    bool flip = bestDot < 0;

    if (!flip)
    {
        outTop[0] = 0;
        outTop[1] = 1;
        outTop[2] = 2;
        outTop[3] = 3;
    }
    else
    {
        outTop[0] = 4;
        outTop[1] = 5;
        outTop[2] = 6;
        outTop[3] = 7;
    }
}


inline void DebugPrintOBB(const OBB& obb, const std::string& name = "")
{
    glm::vec3 axes[3];
    GetOBBAxes(obb, axes);

    glm::vec3 corners[8];
    int idx = 0;

    for (int x = -1; x <= 1; x += 2)
    for (int y = -1; y <= 1; y += 2)
    for (int z = -1; z <= 1; z += 2)
    {
        corners[idx++] =
            obb.center +
            axes[0] * obb.halfSize.x * (float)x +
            axes[1] * obb.halfSize.y * (float)y +
            axes[2] * obb.halfSize.z * (float)z;
    }

    std::cout << "\n==== OBB DEBUG : " << name << " ====\n";
    std::cout << "Center: " 
              << obb.center.x << ", " << obb.center.y << ", " << obb.center.z << "\n";
    std::cout << "HalfSize: "
              << obb.halfSize.x << ", " << obb.halfSize.y << ", " << obb.halfSize.z << "\n";

    for (int i = 0; i < 8; i++)
    {
        std::cout << "Corner[" << i << "] = ("
                  << corners[i].x << ", "
                  << corners[i].y << ", "
                  << corners[i].z << ")\n";
    }
}
inline void DebugDrawOBB(const OBB& obb, Shader& shader, 
                         const glm::mat4& view,
                         const glm::mat4& projection)
{
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setMat4("model", glm::mat4(1.0f)); // must be identity!

    shader.setVec3("color", glm::vec3(1,0,0));

    glm::vec3 X = obb.orientation[0] * obb.halfSize.x;
    glm::vec3 Y = obb.orientation[1] * obb.halfSize.y;
    glm::vec3 Z = obb.orientation[2] * obb.halfSize.z;

    glm::vec3 c = obb.center;

    glm::vec3 corners[8] = {
        c +  X +  Y +  Z,
        c +  X +  Y -  Z,
        c +  X -  Y +  Z,
        c +  X -  Y -  Z,
        c -  X +  Y +  Z,
        c -  X +  Y -  Z,
        c -  X -  Y +  Z,
        c -  X -  Y -  Z,
    };

    GLuint indices[] = {
        0,1, 1,3, 3,2, 2,0,
        4,5, 5,7, 7,6, 6,4,
        0,4, 1,5, 2,6, 3,7
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

