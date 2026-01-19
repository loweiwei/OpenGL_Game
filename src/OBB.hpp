// OBB.hpp
#pragma once
#include <glm/glm.hpp>

// 全專案唯一的 OBB 定義
struct OBB
{
    glm::vec3 center;
    glm::vec3 halfSize;
    glm::mat3 orientation;
};
