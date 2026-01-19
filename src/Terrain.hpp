#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Terrain {
public:
    Terrain(int gridX, int gridZ, float size, float maxH);

    void generateTerrain();
    void draw();

    float getHeight(float worldX, float worldZ) const;
    float getMaxHeight() const { return maxHeight; }
    // 讓 WorldManager 設水邊高度
    void setWaterNorth(const std::vector<float>& s);
    void setWaterSouth(const std::vector<float>& s);
    void setWaterWest (const std::vector<float>& s);
    void setWaterEast (const std::vector<float>& s);

    //mask 系統
    void addFlatArea(float cx, float cz,
                 float radius,
                 float flatHeight,
                 float smoothness);

    void addCrevasse(
        float cx, float cz,      // 中心位置（tile local space）
        float halfWidth,         // 橫向半徑
        float halfLength,        // 縱向半徑
        float depth,             // 裂縫深度
        float angleDeg 
    );
    void addSlope(
        float dirX, float dirZ,
        float strength,
        float centerOffsetX,
        float centerOffsetZ,
        float radius,
        float smoothness
    );
    void addCurvedCrevasse(
        const std::vector<glm::vec2>& points,
        float width,
        float depth
    );
    void addRidge(const std::vector<glm::vec2>& points,
              float width,
              float height);
    const std::vector<float>& getTopEdge()    const { return topEdge; }
    const std::vector<float>& getBottomEdge() const { return bottomEdge; }
    const std::vector<float>& getLeftEdge()   const { return leftEdge; }
    const std::vector<float>& getRightEdge()  const { return rightEdge; }

private:
    int gridX, gridZ;
    float size;
    float maxHeight;
    const int VC = 256;

    std::vector<float> heightMap;

    // 給地形接縫使用（高度）
    std::vector<float> topEdge, bottomEdge, leftEdge, rightEdge;
    // 水的 shape（只用來壓低地形，不做位移）
    std::vector<float> waterNorth, waterSouth, waterWest, waterEast;

    //=====mask 系統
    //crevasseMask
    std::vector<float> crevasseMask;
    float crevasseDepth = 0.0f;

    //slopeMask
    std::vector<float> slopeMask;
    float slopeStrength = 0.0f;

    //flat
    std::vector<float> flatMask;
    std::vector<float> flatHeightMap;
    float t = 0.0f;
    std::vector<float> ridgeMask;
    std::vector<float> ridgeHeightMap;

    float Depth = 0.0f;

    GLuint vao=0,vbo=0,ebo=0;
    int indexCount=0;
};
