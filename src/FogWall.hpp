#pragma once
#include <glm/glm.hpp>
#include "Shader.h"

class FogWall
{
public:
    glm::vec3 position;
    glm::vec3 halfSize;   // ★ 方形空氣塊：半邊長 (hx, hy, hz)

    FogWall(glm::vec3 pos, glm::vec3 hs)
        : position(pos), halfSize(hs)
    {}

    void draw(Shader& shader);
};
