#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Water {
public:
    Water(int gridX, int gridZ, float size);

    const std::vector<float>& getNorthShape() const { return northShape; }
    const std::vector<float>& getSouthShape() const { return southShape; }
    const std::vector<float>& getWestShape()  const { return westShape;  }
    const std::vector<float>& getEastShape()  const { return eastShape;  }

    void generateMesh();
    void draw();

private:
    int gridX, gridZ;
    float size;

    const int VC = 256;

    // XZ offset shapes
    std::vector<float> northShape;
    std::vector<float> southShape;
    std::vector<float> westShape;
    std::vector<float> eastShape;

    GLuint vao = 0, vbo = 0, ebo = 0;
    int indexCount = 0;
};
