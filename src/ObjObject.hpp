#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ObjMesh.hpp"
#include "Shader.h"
#include <iostream>
#include "OBB.hpp"

class ObjObject {
public:
    ObjMesh mesh;

    glm::vec3 position = glm::vec3(0);
    glm::vec3 rotation = glm::vec3(0);
    glm::vec3 scale    = glm::vec3(1);

    glm::vec3 color = glm::vec3(1);

    OBB obbLocal;   // ⭐ local-space OBB（不會變）
    OBB obb;        // ⭐ world-space OBB（每幀更新）

    ObjObject(const std::string& path, glm::vec3 col = glm::vec3(1))
        : mesh(path), color(col)
    {
        computeLocalOBB();
    }

    // ==========================================================
    // ⭐ Step 1 — 計算 local-space OBB
    // ==========================================================
   void computeLocalOBB()
    {
        if (mesh.vertices.empty()) return;

        glm::vec3 minV( 1e9f), maxV(-1e9f);

        // 直接用 vertices (local-space) 算 AABB
        for (auto& v : mesh.vertices)
        {
            const glm::vec3& p = v.pos;   // ⚠️ 必須是模型 local-space 頂點
            minV = glm::min(minV, p);
            maxV = glm::max(maxV, p);
        }

        // ✅ 正確：AABB 中心（不會被頂點密度偏移）
        obbLocal.center   = (minV + maxV) * 0.5f;
        obbLocal.halfSize = (maxV - minV) * 0.5f;

        obbLocal.orientation = glm::mat3(1.0f);
    }


    void updateWorldOBB()
    {
        glm::mat4 T  = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1,0,0));
        glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0,1,0));
        glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0,0,1));
        glm::mat4 S  = glm::scale(glm::mat4(1.0f), scale);

        glm::mat4 M = T * Rz * Ry * Rx * S;

        // center
        obb.center = glm::vec3(M * glm::vec4(obbLocal.center, 1.0f));

        // axes + scale lengths
        glm::vec3 c0 = glm::vec3(M[0]);
        glm::vec3 c1 = glm::vec3(M[1]);
        glm::vec3 c2 = glm::vec3(M[2]);

        float s0 = glm::length(c0);
        float s1 = glm::length(c1);
        float s2 = glm::length(c2);

        obb.orientation[0] = c0 / (s0 + 1e-8f);
        obb.orientation[1] = c1 / (s1 + 1e-8f);
        obb.orientation[2] = c2 / (s2 + 1e-8f);

        // halfSize：用實際縮放量
        obb.halfSize = obbLocal.halfSize * glm::vec3(s0, s1, s2);
    }



    // ==========================================================
    // Rendering
    // ==========================================================
    void draw(Shader& shader)
    {
        updateWorldOBB();

        shader.use();
        shader.setVec3("color", color);
        shader.setInt("diffuseTex", 0);

        glm::mat4 M(1.0f);

        // rotation
        glm::quat qx = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1,0,0));
        glm::quat qy = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0,1,0));
        glm::quat qz = glm::angleAxis(glm::radians(rotation.z), glm::vec3(0,0,1));
        glm::quat q = qz * qy * qx;
        glm::mat4 R = glm::mat4_cast(q);

        M = glm::translate(M, position) * R;
        M = glm::scale(M, scale);

        shader.setMat4("model", M);

        glDisable(GL_CULL_FACE);
        mesh.draw();
        glEnable(GL_CULL_FACE);
    }
};