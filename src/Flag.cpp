#include "Flag.hpp"
#include <glm/gtc/matrix_transform.hpp>

Flag::Flag(glm::vec3 p, float h)
    : pos(p), height(h)
{
}

void Flag::init()
{
    float vertices[] =
    {
        // 桿 (線段)
        0.0f, 0.0f, 0.0f,
        0.0f, height, 0.0f,

        // 小三角旗 (三角形)
        0.0f, height, 0.0f,
        0.8f, height - 0.4f, 0.0f,
        0.0f, height - 0.8f, 0.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                 vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Flag::draw(Shader& shader)
{
    shader.use();

    // ⭐ 傳顏色
    shader.setVec3("flagColor", color);

    // ⭐ Model
    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    shader.setMat4("model", model);

    glBindVertexArray(vao);

    // 畫旗桿
    glDrawArrays(GL_LINES, 0, 2);

    // 畫小旗
    glDrawArrays(GL_TRIANGLES, 2, 3);
}
