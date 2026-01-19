#include "Cube.hpp"
#include <cstdlib>
#include <ctime>

static float baseVertices[] = {
    // pos                 // normal        // uv
    -0.5f,-0.5f,-0.5f,   0,0,-1,   0,0,
     0.5f,-0.5f,-0.5f,   0,0,-1,   1,0,
     0.5f, 0.5f,-0.5f,   0,0,-1,   1,1,
     0.5f, 0.5f,-0.5f,   0,0,-1,   1,1,
    -0.5f, 0.5f,-0.5f,   0,0,-1,   0,1,
    -0.5f,-0.5f,-0.5f,   0,0,-1,   0,0,

    -0.5f,-0.5f, 0.5f,   0,0,1,   0,0,
     0.5f,-0.5f, 0.5f,   0,0,1,   1,0,
     0.5f, 0.5f, 0.5f,   0,0,1,   1,1,
     0.5f, 0.5f, 0.5f,   0,0,1,   1,1,
    -0.5f, 0.5f, 0.5f,   0,0,1,   0,1,
    -0.5f,-0.5f, 0.5f,   0,0,1,   0,0,

    -0.5f, 0.5f, 0.5f,  -1,0,0,   1,0,
    -0.5f, 0.5f,-0.5f,  -1,0,0,   1,1,
    -0.5f,-0.5f,-0.5f,  -1,0,0,   0,1,
    -0.5f,-0.5f,-0.5f,  -1,0,0,   0,1,
    -0.5f,-0.5f, 0.5f,  -1,0,0,   0,0,
    -0.5f, 0.5f, 0.5f,  -1,0,0,   1,0,

     0.5f, 0.5f, 0.5f,   1,0,0,   1,0,
     0.5f, 0.5f,-0.5f,   1,0,0,   1,1,
     0.5f,-0.5f,-0.5f,   1,0,0,   0,1,
     0.5f,-0.5f,-0.5f,   1,0,0,   0,1,
     0.5f,-0.5f, 0.5f,   1,0,0,   0,0,
     0.5f, 0.5f, 0.5f,   1,0,0,   1,0,

    -0.5f,-0.5f,-0.5f,   0,-1,0,   0,1,
     0.5f,-0.5f,-0.5f,   0,-1,0,   1,1,
     0.5f,-0.5f, 0.5f,   0,-1,0,   1,0,
     0.5f,-0.5f, 0.5f,   0,-1,0,   1,0,
    -0.5f,-0.5f, 0.5f,   0,-1,0,   0,0,
    -0.5f,-0.5f,-0.5f,   0,-1,0,   0,1,

    -0.5f, 0.5f,-0.5f,   0,1,0,   0,1,
     0.5f, 0.5f,-0.5f,   0,1,0,   1,1,
     0.5f, 0.5f, 0.5f,   0,1,0,   1,0,
     0.5f, 0.5f, 0.5f,   0,1,0,   1,0,
    -0.5f, 0.5f, 0.5f,   0,1,0,   0,0,
    -0.5f, 0.5f,-0.5f,   0,1,0,   0,1,
};

Cube::Cube(bool deform)
{
    //std::srand(std::time(nullptr));

    // 1. 複製基本頂點
    for (int i = 0; i < 36 * 8; i++)
        vertices[i] = baseVertices[i];

    if (deform)
    {
        // 2. 產生 8 個角點各自的偏移
        glm::vec3 offset[8];
        for (int i = 0; i < 8; i++) {
            offset[i].x = ((rand() % 100) / 100.0f - 0.5f) * 0.15f;
            offset[i].y = ((rand() % 100) / 100.0f - 0.5f) * 0.10f;
            offset[i].z = ((rand() % 100) / 100.0f - 0.5f) * 0.15f;
        }

        // 3. 每個 vertex 對應到哪個角點 → 套用同一組 offset
        for (int i = 0; i < 36; i++)
        {
            int idx = i * 8;
            float x = vertices[idx+0];
            float y = vertices[idx+1];
            float z = vertices[idx+2];

            int cornerIndex =
                (x > 0 ? 1 : 0) +
                (y > 0 ? 2 : 0) +
                (z > 0 ? 4 : 0);

            vertices[idx+0] += offset[cornerIndex].x;
            vertices[idx+1] += offset[cornerIndex].y;
            vertices[idx+2] += offset[cornerIndex].z;
        }
    }

    // ====== 建 VAO / VBO ========
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Cube::~Cube()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Cube::draw()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
