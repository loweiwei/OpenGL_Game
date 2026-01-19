// ===========================
// RobotModel.hpp  (FULL)
// ===========================
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "Shader.h"

// ---- 關節資料（含限制）----
struct Joint {
    float x = 0, y = 0, z = 0;
    float minX = -180, maxX = 180;
    float minY = -180, maxY = 180;
    float minZ = -180, maxZ = 180;
};

enum class RobotAction {
    Stop = 0,
    Walk,
    Run,
    Bow
};

struct SatelliteState {
    bool revolutionOn = true;   // 公轉
    bool rotationOn   = true;   // 自轉
    float radius      = 2.8f;   // 軌道半徑
    float revSpeed    = 1.2f;   // 公轉速度(rad/s)
    float rotSpeed    = 2.5f;   // 自轉速度(rad/s)
    glm::vec3 axis    = glm::vec3(0,1,0); // 自轉軸 (X/Y/Z)
};

class RobotModel {
public:
    // 人形關節（至少 2 DOF：肩/髖；肘/膝 1 DOF）
    Joint head;                 // head: y(左右) + x(點頭)

    Joint shoulderL, elbowL;
    Joint shoulderR, elbowR;

    Joint hipL, kneeL;
    Joint hipR, kneeR;

    // menu / control state
    int selected = 0;           // 0..8
    RobotAction action = RobotAction::Stop;
    SatelliteState sat;

    void selectNext() { selected = (selected + 1) % 9; }
    void selectPrev() { selected = (selected + 8) % 9; }

    void addAngle(char axis, float d);
    void draw(Shader& shader, const glm::mat4& base, float t);

private:
    Joint& selJoint();
};
