#pragma once
#include <glm/glm.hpp>
#include <vector>

enum Direction { NORTH, SOUTH, EAST, WEST };

class IceMountain
{
public:
    IceMountain(glm::vec3 position,
                float radius,
                float height,
                Direction doorA,
                Direction doorB);

    

    void generateMesh();     // 生成大冰山 mesh（含雙洞）
    void draw();             // OpenGL draw

    glm::vec3 position;
    float radius;
    float height;

    Direction frontDoor;     // 洞口1
    Direction backDoor;      // 洞口2
    float rotationY = 0.0f;
    //float getHeight(float worldX, float worldZ);

private:
    unsigned int vao = 0, vbo = 0, ebo = 0;
    int indexCount = 0;
    void computeNormals(std::vector<float>& vertices,
                           const std::vector<unsigned int>& indices);
    void addVertex(std::vector<float>& v,
                   glm::vec3 pos, glm::vec3 normal, glm::vec2 uv);

    void generateIceShape(std::vector<float>& vertices,
                          std::vector<unsigned int>& indices);

    void carveDoor(std::vector<float>& vertices,
                   Direction dir);
};
