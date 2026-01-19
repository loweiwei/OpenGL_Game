#pragma once
#include "SmallObject.hpp"
#include <vector>

// 共用：三角形 (pos + normal)
static void pushTri(std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
    float t[] = {
        a.x,a.y,a.z, n.x,n.y,n.z,
        b.x,b.y,b.z, n.x,n.y,n.z,
        c.x,c.y,c.z, n.x,n.y,n.z
    };
    v.insert(v.end(), t, t+18);
}

class Tent : public SmallObject {
public:
    glm::vec3 clothColor;
    glm::vec3 poleColor;

    Tent(glm::vec3 pos, glm::vec3 scl,
         glm::vec3 clothCol, glm::vec3 poleCol)
        : SmallObject(pos, scl, clothCol),
          clothColor(clothCol), poleColor(poleCol) {}

    // ============ 可調參數 ==================
    float clothW = 8.0f; 
    float clothH = 3.0f;
    float clothZ = 2.5f;

    float poleH  = 3.0f;
    float poleX  = 1.0f;
    float poleR  = 0.08f;

private:
    // === VAO / VBO ===
    unsigned int vaoLeft,  vboLeft;
    unsigned int vaoRight, vboRight;
    unsigned int vaoPoleL, vboPoleL;
    unsigned int vaoPoleR, vboPoleR;

    // === Count (int! 不可用 unsigned!) ===
    int countLeft  = 0;
    int countRight = 0;
    int countPoleL = 0;
    int countPoleR = 0;

public:

    void init()
    {
        // ------------------------
        // 左布 (Rectangle)
        // ------------------------
        std::vector<float> L;

        glm::vec3 TL(-clothW/2, poleH, 0);
        glm::vec3 BL(-clothW/2, 0,      clothZ);
        glm::vec3 TR(0,          poleH, 0);
        glm::vec3 BR(0,          0,      clothZ);

        pushTri(L, TL, TR, BR);
        pushTri(L, TL, BR, BL);

        buildVAO(vaoLeft, vboLeft, L, countLeft);

        // ------------------------
        // 右布
        // ------------------------
        std::vector<float> R;

        glm::vec3 TL2(0,          poleH, 0);
        glm::vec3 BL2(0,          0,      clothZ);
        glm::vec3 TR2(clothW/2,   poleH, 0);
        glm::vec3 BR2(clothW/2,   0,      clothZ);

        pushTri(R, TL2, BR2, TR2);
        pushTri(R, TL2, BL2, BR2);

        buildVAO(vaoRight, vboRight, R, countRight);

        // ------------------------
        // 左柱
        // ------------------------
        std::vector<float> PL;
        addPole(PL, glm::vec3(-poleX, 0, 0));
        buildVAO(vaoPoleL, vboPoleL, PL, countPoleL);

        // ------------------------
        // 右柱
        // ------------------------
        std::vector<float> PR;
        addPole(PR, glm::vec3(+poleX, 0, 0));
        buildVAO(vaoPoleR, vboPoleR, PR, countPoleR);
    }

    // ----------- 加一根四面柱子 -----------
    void addPole(std::vector<float>& v, glm::vec3 base)
    {
        glm::vec3 A = base + glm::vec3(-poleR, 0, 0);
        glm::vec3 B = base + glm::vec3(+poleR, 0, 0);
        glm::vec3 C = base + glm::vec3(-poleR, poleH, 0);
        glm::vec3 D = base + glm::vec3(+poleR, poleH, 0);

        pushTri(v, A, C, B);
        pushTri(v, B, C, D);
    }

    // ----------- 建 VAO -----------
    void buildVAO(unsigned int& vao, unsigned int& vbo,
                  std::vector<float>& buf, int& count)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     buf.size()*sizeof(float),
                     buf.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              6*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        count = buf.size() / 6;
    }

    // ----------- draw -----------
    void draw(Shader& shader) override
    {
        shader.use();

        glm::mat4 M = glm::translate(glm::mat4(1), position);
        M = glm::scale(M, scale);
        shader.setMat4("model", M);

        // 左布
        shader.setVec3("color", clothColor);
        glBindVertexArray(vaoLeft);
        glDrawArrays(GL_TRIANGLES, 0, countLeft);

        // 右布
        glBindVertexArray(vaoRight);
        glDrawArrays(GL_TRIANGLES, 0, countRight);

        // 左柱
        shader.setVec3("color", poleColor);
        glBindVertexArray(vaoPoleL);
        glDrawArrays(GL_TRIANGLES, 0, countPoleL);

        // 右柱
        glBindVertexArray(vaoPoleR);
        glDrawArrays(GL_TRIANGLES, 0, countPoleR);
    }
};
