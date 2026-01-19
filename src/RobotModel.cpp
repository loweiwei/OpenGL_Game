// ============================================================
// RobotModel.cpp  (FULL / PIVOT-CORRECT + DEBUG VERSION)
// ✅ lowerArm / lowerLeg 旋轉圓心 = upperArm / upperLeg 的末端 (elbow/knee pivot)
// ✅ pivot matrices never scaled (translate+rotate only)
// ✅ bone mesh extended with overlap, BUT child pivot uses nominalLen (no drift)
// ✅ optional debug: draw tiny cubes at pivots + bone end pivots
// ============================================================

#include "RobotModel.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <algorithm>

extern unsigned int gCubeVAO;

static void drawCube(Shader& shader, const glm::mat4& m)
{
    shader.setMat4("model", m);
    glBindVertexArray(gCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void drawSatellite(Shader& shader, const glm::mat4& m)
{
    drawCube(shader, glm::scale(m, glm::vec3(0.22f)));
}

Joint& RobotModel::selJoint()
{
    switch (selected) {
    case 0: return head;
    case 1: return shoulderL;
    case 2: return elbowL;
    case 3: return shoulderR;
    case 4: return elbowR;
    case 5: return hipL;
    case 6: return kneeL;
    case 7: return hipR;
    default: return kneeR; // 8
    }
}

void RobotModel::addAngle(char axis, float d)
{
    Joint& j = selJoint();
    if (axis == 'x') j.x = std::clamp(j.x + d, j.minX, j.maxX);
    if (axis == 'y') j.y = std::clamp(j.y + d, j.minY, j.maxY);
    if (axis == 'z') j.z = std::clamp(j.z + d, j.minZ, j.maxZ);
}

void RobotModel::draw(Shader& shader, const glm::mat4& base, float t)
{
    // =====================================================
    // 0) action -> auto swing
    // =====================================================
    float amp = 0.0f, spd = 0.0f;
    if (action == RobotAction::Walk) { amp = 22.0f; spd = 3.0f; }
    if (action == RobotAction::Run ) { amp = 32.0f; spd = 6.0f; }

    float autoLeg = std::sin(t * spd) * amp;
    float autoArm = std::sin(t * spd + 3.1415926f) * (amp * 0.7f);

    // =====================================================
    // ⭐ Bow(雙手合併 + 腰前彎) 動作參數（只在 action==Bow 生效）
    // =====================================================
    float bowBodyLeanX      = 0.0f;   // 上半身繞腰前彎（X）
    float bowBreathX        = 0.0f;   // 微呼吸
    float bowArmRaiseX      = 0.0f;   // upperArm 抬起（X）
    float bowArmInYawY      = 0.0f;   // upperArm 往中心夾（Y）
    float bowArmInRollZ     = 0.0f;   // upperArm 內收/合掌姿（Z）
    float bowArmSwayY       = 0.0f;   // 小輕擺（Y）
    float bowElbowBendX     = 0.0f;   // elbow 彎曲（X）
    float bowElbowInZ       = 0.0f;   // lowerarm 內轉（Z）
    float bowHandInX        = 0.0f;   // 手掌往中心再推一點（讓兩手更靠近）
    float bowHeadDownX      = 0.0f;   // 頭微低

    if (action == RobotAction::Bow)
    {
        // Bow 不要走路擺動
        autoLeg = 0.0f;
        autoArm = 0.0f;

        // 速度：小幅擺動不要太快
        float bSpd = 4.8f;
        float w    = std::sin(t * bSpd);
        float w2   = std::sin(t * bSpd + 1.2f);

        // ✅腰前彎（在腿跟身體連接處）
        // ✅✅✅ 腰部「回正 ↔ 前傾」擺動（不會後仰）
        // bowPhase: 0=回正, 1=最前傾
        float bowSpd = 2.6f; // 鞠躬節奏：2~4 好看
        float bowPhase = 0.5f * (1.0f + std::sin(t * bowSpd)); // 0..1

        // 讓動作更像鞠躬：回正停一下、下去更乾脆（smoothstep）
        bowPhase = bowPhase * bowPhase * (3.0f - 2.0f * bowPhase);

        float bowLeanMax = -18.0f; // 最大前傾角：12~25 自己調
        bowBodyLeanX = -bowLeanMax * bowPhase; // ✅只會在 [0, -bowLeanMax] 擺動（回正->前傾->回正）

        // 微呼吸（很小就好，避免影響鞠躬）
        bowBreathX = -1.2f * std::sin(t * 2.0f);


        // ✅雙手合併：upperArm 先抬起、再往中間夾
        bowArmRaiseX  = -52.0f + 2.5f * w;   // upperArm 向上抬（其實是前上舉）
        bowArmInYawY  =  28.0f;              // 往中心夾（越大越靠中）
        bowArmInRollZ =  18.0f;              // 讓上臂姿勢更像合掌

        // ✅lowerArm：先內轉一點 + 彎起來
        bowElbowBendX = -78.0f + 3.0f * w2;  // 彎曲讓手上來
        bowElbowInZ   =  16.0f;              // 內轉（12~30 自己調）

        // ✅手再往中心推一點（讓兩手碰到一起）
        bowHandInX = 0.07f;                  // 0.06~0.14 自己調

        // ✅小輕擺（整段手臂）
        bowArmSwayY = 6.0f * w;              // 小幅左右晃

        // ✅頭微低（更像鞠躬）
        bowHeadDownX = -8.0f;
    }

    float pulse = 1.0f + 0.06f * std::sin(t * 6.2831853f);
    auto maybePulse = [&](int jointIndex) { return (selected == jointIndex) ? pulse : 1.0f; };

    // =====================================================
    // 1) 尺寸/位置參數（調整都在這）
    // =====================================================
    // torso
    const float torsoW = 0.78f, torsoH = 0.80f, torsoD = 0.55f;
    const float hipY   = 0.25f;

    // head/neck
    const float headS   = 0.52f;
    const float neckLen = 0.18f;

    // arms
    const float upperArmLen = 0.44f, lowerArmLen = 0.36f;
    const float upperArmThk = 0.18f, lowerArmThk = 0.16f;
    const float handW = 0.20f, handH = 0.12f, handD = 0.20f;

    // legs
    const float upperLegLen = 0.45f, lowerLegLen = 0.45f;
    const float upperLegThk = 0.22f, lowerLegThk = 0.20f;
    const float shoeW = 0.30f, shoeH = 0.14f, shoeD = 0.40f;

    // joint visual + overlap
    const float jointR  = 0.09f;
    const float overlap = 0.05f;

    // offsets
    const float neckUp     = 0.06f;
    const float shoulderUp = 0.20f;
    const float hipUp      = 0.10f;

    // spacing
    const float shoulderOut = 0.22f;
    const float hipOut      = 0.14f;

    // leg spread
    const float hipSpread   = 0.48f;

    // =====================================================
    // ⭐ 顏色工具 + 配色（只新增顏色，不動其他）
    // =====================================================
    auto setColor = [&](const glm::vec3& c) {
        shader.setVec3("baseColor", c);
    };

    const glm::vec3 COL_TORSO   = {0.20f, 0.55f, 0.95f};
    const glm::vec3 COL_BELLY   = {0.10f, 0.35f, 0.75f};

    const glm::vec3 COL_NECK    = {0.75f, 0.75f, 0.80f};
    const glm::vec3 COL_HEAD    = {0.85f, 0.85f, 0.90f};

    const glm::vec3 COL_JOINT   = {0.95f, 0.95f, 0.95f};

    const glm::vec3 COL_UP_ARM  = {0.95f, 0.65f, 0.20f};
    const glm::vec3 COL_LO_ARM  = {0.90f, 0.50f, 0.10f};
    const glm::vec3 COL_HAND    = {0.70f, 0.70f, 0.75f};

    const glm::vec3 COL_UP_LEG  = {0.25f, 0.85f, 0.45f};
    const glm::vec3 COL_LO_LEG  = {0.15f, 0.70f, 0.35f};
    const glm::vec3 COL_SHOE    = {0.12f, 0.12f, 0.12f};

    const glm::vec3 COL_EYE     = {0.0f, 0.0f, 0.0f};
    const glm::vec3 COL_MOUTH   = {1.00f, 0.45f, 0.75f};

    const glm::vec3 COL_ANTENNA = {0.90f, 0.90f, 0.95f};
    const glm::vec3 COL_ANTBALL = {1.00f, 0.30f, 0.30f};

    const glm::vec3 COL_SAT     = {0.95f, 0.25f, 0.95f};

    // =====================================================
    // 2) 推導
    // =====================================================
    const float torsoHalfW = torsoW * 0.5f;
    const float torsoHalfH = torsoH * 0.5f;
    const float torsoTop   = +torsoHalfH;
    const float torsoBot   = -torsoHalfH;

    // =====================================================
    // 3) DEBUG 開關
    // =====================================================
    const bool  DBG_PIVOT_MARKERS = true;
    const bool  DBG_BONE_ENDS     = true;
    const float DBG_MARKER_SIZE   = 0.035f;

    auto drawMarker = [&](const glm::mat4& P)
    {
        if (!DBG_PIVOT_MARKERS) return;
        drawCube(shader, glm::scale(P, glm::vec3(DBG_MARKER_SIZE)));
    };

    auto drawEndMarker = [&](const glm::mat4& P)
    {
        if (!DBG_BONE_ENDS) return;
        drawCube(shader, glm::scale(P, glm::vec3(DBG_MARKER_SIZE * 0.85f)));
    };

    // =====================================================
    // 4) 小工具：球 / 連接帶 / 骨頭（pivot 永不漂）
    // =====================================================
    auto drawJointBall = [&](const glm::mat4& pivotM, float mul = 1.0f)
    {
        setColor(COL_JOINT);
        drawCube(shader, glm::scale(pivotM, glm::vec3(jointR * mul)));
    };

    auto drawConnectorX = [&](const glm::mat4& torsoBaseM, float yLocal,
                              float xFrom, float xTo, float thickY, float thickZ)
    {
        float cx  = 0.5f * (xFrom + xTo);
        float len = std::fabs(xTo - xFrom) + jointR * 0.80f;
        glm::mat4 C = glm::translate(torsoBaseM, glm::vec3(cx, yLocal, 0.0f));
        C = glm::scale(C, glm::vec3(len, thickY, thickZ));
        drawCube(shader, C);
    };

    auto drawBoneDown = [&](const glm::mat4& pivotM, float lenNominal, float thk, float overlapAmt) -> glm::mat4
    {
        const float segLen = lenNominal + 2.0f * overlapAmt;

        glm::mat4 B = glm::translate(pivotM, glm::vec3(0, -(lenNominal), 0));
        B = glm::scale(B, glm::vec3(thk, segLen, thk));
        drawCube(shader, B);

        glm::mat4 endPivot = glm::translate(pivotM, glm::vec3(0, -lenNominal, 0));
        drawEndMarker(endPivot);
        return endPivot;
    };

    auto drawBoneUp = [&](const glm::mat4& pivotM, float lenNominal, float thk, float overlapAmt) -> glm::mat4
    {
        const float segLen = lenNominal + 2.0f * overlapAmt;

        glm::mat4 B = glm::translate(pivotM, glm::vec3(0, +(lenNominal * 0.5f), 0));
        B = glm::scale(B, glm::vec3(thk, segLen, thk));
        drawCube(shader, B);

        glm::mat4 endPivot = glm::translate(pivotM, glm::vec3(0, +lenNominal, 0));
        drawEndMarker(endPivot);
        return endPivot;
    };

    // =====================================================
    // ✅ 5) pelvisBase(腿用) + upperBase(上半身用)
    // =====================================================
    glm::mat4 pelvisBase = glm::translate(base, glm::vec3(0, hipY, 0));

    glm::mat4 upperBase = pelvisBase;
    if (action == RobotAction::Bow) {
        upperBase = glm::rotate(upperBase, glm::radians(bowBodyLeanX + bowBreathX), glm::vec3(1,0,0));
    }

    // =====================================================
    // 6) Torso（上半身用 upperBase）
    // =====================================================
    setColor(COL_TORSO);
    drawCube(shader, glm::scale(upperBase, glm::vec3(torsoW, torsoH, torsoD)));

    // belly
    {
        glm::mat4 belly = glm::translate(upperBase, glm::vec3(0.0f, 0.05f, 0.30f));
        belly = glm::scale(belly, glm::vec3(0.45f, 0.32f, 0.18f));
        setColor(COL_BELLY);
        drawCube(shader, belly);
    }

    // =====================================================
    // 7) Neck + Head（上半身用 upperBase）
    // =====================================================
    const float neckEmbed = jointR * 0.35f;
    glm::mat4 neckPivot = glm::translate(upperBase, glm::vec3(0, torsoTop - neckEmbed + neckUp, 0));
    drawMarker(neckPivot);
    drawJointBall(neckPivot, 1.0f);

    setColor(COL_NECK);
    glm::mat4 neckTopPivot = drawBoneUp(neckPivot, neckLen, 0.18f, overlap);

    glm::mat4 headPivot = glm::translate(neckTopPivot, glm::vec3(0, headS * 0.5f - overlap, 0));

    // ✅Bow 頭微低
    float headX = head.x + bowHeadDownX;
    headPivot = glm::rotate(headPivot, glm::radians(head.y), glm::vec3(0,1,0));
    headPivot = glm::rotate(headPivot, glm::radians(headX),     glm::vec3(1,0,0));

    setColor(COL_HEAD);
    drawCube(shader, glm::scale(headPivot, glm::vec3(headS * maybePulse(0))));

    // face (兩眼 + 嘴) 你要「往上移」就是改這裡 y 值
    setColor(COL_EYE);
    drawCube(shader, glm::scale(glm::translate(headPivot, glm::vec3( 0.14f, 0.12f, 0.28f)), glm::vec3(0.09f, 0.12f, 0.06f)));
    drawCube(shader, glm::scale(glm::translate(headPivot, glm::vec3(-0.14f, 0.12f, 0.28f)), glm::vec3(0.09f, 0.12f, 0.06f)));

    setColor(COL_MOUTH);
    drawCube(shader, glm::scale(glm::translate(headPivot, glm::vec3(0, -0.03f, 0.28f)), glm::vec3(0.14f, 0.05f, 0.05f)));

    // antenna
    {
        glm::mat4 ant = glm::translate(headPivot, glm::vec3(0.0f, 0.42f, 0.0f));
        ant = glm::scale(ant, glm::vec3(0.06f, 0.22f, 0.06f));
        setColor(COL_ANTENNA);
        drawCube(shader, ant);

        glm::mat4 ball = glm::translate(headPivot, glm::vec3(0.0f, 0.58f, 0.0f));
        ball = glm::scale(ball, glm::vec3(0.11f));
        setColor(COL_ANTBALL);
        drawCube(shader, ball);
    }

    // =====================================================
    // 8) Arms（上半身用 upperBase）
    // =====================================================
    auto buildArm = [&](int side, Joint& shoulder, Joint& elbow,
                        float autoSwingDeg, int shoulderIdx, int elbowIdx)
    {
        float shoulderYLocal = torsoTop - (jointR + upperArmThk * 0.35f) + shoulderUp;
        float pivotX = (torsoHalfW + shoulderOut) * (float)side;

        float torsoSurfaceX = torsoHalfW * (float)side;
        setColor(COL_TORSO);
        drawConnectorX(upperBase, shoulderYLocal, torsoSurfaceX, pivotX,
                       upperArmThk * 0.55f, upperArmThk * 0.85f);

        glm::mat4 A = glm::translate(upperBase, glm::vec3(pivotX, shoulderYLocal, 0.0f));
        drawMarker(A);

        // ✅Bow：upperArm 抬高 + 往中心夾 + 小擺動
        float shX = shoulder.x + autoSwingDeg;
        float shY = shoulder.y;
        float shZ = shoulder.z;

        if (action == RobotAction::Bow) {
            shX += bowArmRaiseX;
            shY += (-side) * bowArmInYawY + bowArmSwayY; // 左右相反往中心夾
            shZ += (-side) * bowArmInRollZ;              // 左右相反內收
        }

        A = glm::rotate(A, glm::radians(shY), glm::vec3(0,1,0));
        A = glm::rotate(A, glm::radians(shZ), glm::vec3(0,0,1));
        A = glm::rotate(A, glm::radians(shX), glm::vec3(1,0,0));

        drawJointBall(A, maybePulse(shoulderIdx));

        setColor(COL_UP_ARM);
        glm::mat4 elbowPivot = drawBoneDown(A, upperArmLen, upperArmThk * maybePulse(shoulderIdx), overlap);
        drawMarker(elbowPivot);
        drawJointBall(elbowPivot, maybePulse(elbowIdx));

        // ✅Bow：lowerArm 先內轉一點 + 彎曲
        glm::mat4 E = elbowPivot;

        float elX = elbow.x;
        float elZ = 0.0f;
        if (action == RobotAction::Bow) {
            elX += bowElbowBendX;
            elZ += (-side) * bowElbowInZ; // 左右相反內轉
        }

        E = glm::rotate(E, glm::radians(elX), glm::vec3(1,0,0));
        E = glm::rotate(E, glm::radians(elZ), glm::vec3(0,0,1));

        setColor(COL_LO_ARM);
        glm::mat4 wristPivot = drawBoneDown(E, lowerArmLen, lowerArmThk * maybePulse(elbowIdx), overlap);

        // ✅Bow：手掌再往中心推，讓雙手「合併」
        glm::vec3 handLocal = glm::vec3(0, -0.05f, 0.06f);
        if (action == RobotAction::Bow) {
            handLocal.x += (-side) * bowHandInX; // 往中心推
        }

        glm::mat4 hand = glm::translate(wristPivot, handLocal);
        hand = glm::scale(hand, glm::vec3(handW, handH, handD));
        setColor(COL_HAND);
        drawCube(shader, hand);
    };

    buildArm(-1, shoulderL, elbowL,  autoArm, 1, 2);
    buildArm(+1, shoulderR, elbowR, -autoArm, 3, 4);

    // =====================================================
    // 9) Legs（腿用 pelvisBase：不跟著上半身彎）
    // =====================================================
    auto buildLeg = [&](int side, Joint& hip, Joint& knee,
                        float autoSwingDeg, int hipIdx, int kneeIdx)
    {
        float hipYLocal = torsoBot + (jointR + upperLegThk * 0.30f) + hipUp;

        float hipXBase = torsoHalfW * hipSpread;
        float pivotX   = (hipXBase + hipOut) * (float)side;

        float pelvisSurfaceX = hipXBase * (float)side;
        setColor(COL_TORSO);
        drawConnectorX(pelvisBase, hipYLocal, pelvisSurfaceX, pivotX,
                       upperLegThk * 0.55f, upperLegThk * 0.85f);

        glm::mat4 L = glm::translate(pelvisBase, glm::vec3(pivotX, hipYLocal, 0.0f));
        drawMarker(L);

        L = glm::rotate(L, glm::radians(hip.y), glm::vec3(0,1,0));
        L = glm::rotate(L, glm::radians(hip.z), glm::vec3(0,0,1));
        L = glm::rotate(L, glm::radians(hip.x + autoSwingDeg), glm::vec3(1,0,0));

        drawJointBall(L, maybePulse(hipIdx));

        setColor(COL_UP_LEG);
        glm::mat4 kneePivot = drawBoneDown(L, upperLegLen, upperLegThk * maybePulse(hipIdx), overlap);
        drawMarker(kneePivot);
        drawJointBall(kneePivot, maybePulse(kneeIdx));

        glm::mat4 K = kneePivot;
        K = glm::rotate(K, glm::radians(knee.x), glm::vec3(1,0,0));

        setColor(COL_LO_LEG);
        glm::mat4 anklePivot = drawBoneDown(K, lowerLegLen, lowerLegThk * maybePulse(kneeIdx), overlap);

        glm::mat4 shoe = glm::translate(anklePivot, glm::vec3(0, -0.04f, 0.12f));
        shoe = glm::scale(shoe, glm::vec3(shoeW, shoeH, shoeD));
        setColor(COL_SHOE);
        drawCube(shader, shoe);
    };

    buildLeg(-1, hipL, kneeL,  autoLeg, 5, 6);
    buildLeg(+1, hipR, kneeR, -autoLeg, 7, 8);

    // =====================================================
    // 10) Satellite（跟著上半身 upperBase）
    // =====================================================
    float rev = sat.revolutionOn ? (t * sat.revSpeed) : 0.0f;
    float rot = sat.rotationOn   ? (t * sat.rotSpeed) : 0.0f;

    glm::mat4 S = upperBase;
    S = glm::translate(S, glm::vec3(0, 0.55f, 0));
    S = glm::rotate(S, rev, glm::vec3(0,1,0));
    S = glm::translate(S, glm::vec3(sat.radius, 0, 0));
    S = glm::rotate(S, rot, sat.axis);
    setColor(COL_SAT);
    drawSatellite(shader, S);
}
