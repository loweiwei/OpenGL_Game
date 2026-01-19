#include "Aurora.h"
#include <glm/gtc/matrix_transform.hpp>

Aurora::Aurora()
    : shader("shaders/aurora.vert", "shaders/aurora.frag")
{
    float vertices[] = {
        // position            // uv
        -500.0f, 200.0f, -500.0f,  0.0f, 0.0f,
         500.0f, 200.0f, -500.0f,  1.0f, 0.0f,
         500.0f, 200.0f,  500.0f,  1.0f, 1.0f,
        -500.0f, 200.0f,  500.0f,  0.0f, 1.0f
    };

    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    unsigned int ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}


void Aurora::draw(Camera& camera,
                  const glm::mat4& projection,
                  float time)
{
    shader.use();

    glm::mat4 model = glm::translate(glm::mat4(1.0f),
        glm::vec3(camera.Position.x, 0.0f, camera.Position.z));

    shader.setMat4("model", model);
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("projection", projection);
    shader.setFloat("time", time);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
