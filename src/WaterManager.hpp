#pragma once
#include <vector>
#include "WaterSurface.hpp"

class Terrain;

class WaterManager {
public:
    std::vector<WaterSurface*> waters;

    void add(WaterSurface* w) {
        waters.push_back(w);
    }

    void draw();
    
    // add functions
    void addWaterPuddle(Terrain* terrain, float cx, float cz, float r, float offset);
    void addLake(Terrain* terrain, float cx, float cz, float r, float offset);
    void addRiver(Terrain* terrain, const std::vector<glm::vec2>& pts, float width, float offset);
    void addCrevasseWater(Terrain* terrain, float cx, float cz, float width, float depth);
    void addEllipseLake(Terrain* terrain,float cx, float cz,float radiusX, float radiusY,float offset);
    void generateAll(const Terrain& terrain);
};
