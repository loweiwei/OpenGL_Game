#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.h"

class Flag {
public:
    glm::vec3 pos;
    float height;

    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);   // ⭐ 白色

    Flag(glm::vec3 p, float h = 14.0f);

    void init();
    void draw(Shader& shader);   // ⭐ 改成用 shader
private:
    GLuint vao = 0, vbo = 0;
};
