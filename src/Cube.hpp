#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Cube {
public:
    Cube(bool deform = false);  // deform=true → 不規則破碎浮冰
    ~Cube();

    void draw();

private:
    GLuint VAO, VBO;
    float vertices[36 * 8]; // 36 個三角形頂點，每個頂點 8 attributes
    void deformShape();     // ★ 新增：破碎變形
};
