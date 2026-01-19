#include "FogWall.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>

static unsigned int cubeVAO = 0;
static unsigned int cubeVBO = 0;
static unsigned int cubeEBO = 0;
static int cubeIndexCount = 0;

// =====================================================
// 建立 unit cube（只做一次）
// 頂點在 [-0.5, 0.5]
// =====================================================
static void buildCube()
{
    if (cubeVAO != 0) return;

    // 8 vertices
    glm::vec3 verts[] = {
        {-0.5f,-0.5f,-0.5f}, // 0
        { 0.5f,-0.5f,-0.5f}, // 1
        { 0.5f, 0.5f,-0.5f}, // 2
        {-0.5f, 0.5f,-0.5f}, // 3
        {-0.5f,-0.5f, 0.5f}, // 4
        { 0.5f,-0.5f, 0.5f}, // 5
        { 0.5f, 0.5f, 0.5f}, // 6
        {-0.5f, 0.5f, 0.5f}  // 7
    };

    // 12 triangles = 36 indices
    unsigned int idx[] = {
        // back (-z)
        0,1,2,  0,2,3,
        // front (+z)
        4,6,5,  4,7,6,
        // left (-x)
        0,3,7,  0,7,4,
        // right (+x)
        1,5,6,  1,6,2,
        // bottom (-y)
        0,4,5,  0,5,1,
        // top (+y)
        3,2,6,  3,6,7
    };

    cubeIndexCount = (int)(sizeof(idx) / sizeof(idx[0]));

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

// =====================================================
// Draw Box Fog
// =====================================================
void FogWall::draw(Shader& shader)
{
    buildCube();

    glm::mat4 model(1.0f);
    model = glm::translate(model, position);

    // unit cube [-0.5,0.5] → 乘上完整邊長 (2*halfSize)
    model = glm::scale(model, halfSize * 2.0f);

    shader.use();
    shader.setMat4("model", model);

    // ★ 給 shader 做 ray-box intersection 用
    shader.setVec3("fogCenter", position);
    shader.setVec3("fogHalfSize", halfSize);

    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, cubeIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
