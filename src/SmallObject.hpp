#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

class SmallObject {
protected:
    unsigned int VAO, VBO;
    int vertexCount;

public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;

    SmallObject(glm::vec3 pos, glm::vec3 scl, glm::vec3 col)
        : position(pos), scale(scl), color(col) {}

    virtual ~SmallObject() {}

    virtual void initMesh(float* vertices, int floats) {
        vertexCount = floats / 6;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, floats * sizeof(float), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    virtual void draw(Shader& shader) {
        shader.use();
        shader.setVec3("color", color);

        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, scale);
        shader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
};
