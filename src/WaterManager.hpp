#pragma once

#include <vector>
#include <glm/vec2.hpp>      // glm::vec2
#include "WaterSurface.hpp"

// 前向宣告，避免循環 include
class Terrain;

/**
 * @brief 管理所有水面（湖、河、裂縫水、積水等）。
 * 
 * 負責：
 *  - 保存所有 WaterSurface*
 *  - 提供各種 addXXX() 建立不同形狀的水面
 *  - draw() 統一繪製
 *  - generateAll() 在地形變化後重新生成 mesh
 */
class WaterManager
{
public:
    // 所有水面實例，由 World / Tile 擁有並在 Manager 解構時清理
    std::vector<WaterSurface*> waters;

    // 加入一塊水面
    inline void add(WaterSurface* w)
    {
        if (w)
            waters.push_back(w);
    }

    // 繪製所有水面（呼叫每個 WaterSurface::draw()）
    void draw();

    // --------------------------------------------------
    // 產生各種水面
    // --------------------------------------------------

    // 圓形水坑（水窪）
    void addWaterPuddle(
        Terrain* terrain,
        float cx, float cz,
        float r,
        float offset
    );

    // 圓湖
    void addLake(
        Terrain* terrain,
        float cx, float cz,
        float r,
        float offset
    );

    // 沿 polyline 產生河流
    void addRiver(
        Terrain* terrain,
        const std::vector<glm::vec2>& pts,
        float width,
        float offset
    );

    // 裂縫中的水
    void addCrevasseWater(
        Terrain* terrain,
        float cx, float cz,
        float width,
        float depth
    );

    // 橢圓湖
    void addEllipseLake(
        Terrain* terrain,
        float cx, float cz,
        float radiusX,
        float radiusY,
        float offset
    );

    // 重新根據地形生成所有水面（例如地形更新後）
    void generateAll(const Terrain& terrain);
};
