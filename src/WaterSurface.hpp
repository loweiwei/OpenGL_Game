#pragma once
#include <vector>
#include <glm/glm.hpp>

class Terrain;

class WaterSurface {
public:
    float offset;          // 水位相對地形的 offset
    float sizeX, sizeZ;    // 水面範圍
    float radius = -1.0f; // -1 表示不啟用圓形
    float radiusX = -1.0f;
    float radiusY = -1.0f;
    bool  useEllipse = false;   // 是否啟用橢圓/圓形裁切

    int resolution;        // mesh 細緻度

    unsigned int vao, vbo, ebo;
    int indexCount;

    glm::vec3 origin;      // 水面位置（world space）

    WaterSurface();

    void generateMesh(const Terrain* terrain);
    void draw();
};
