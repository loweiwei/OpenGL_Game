#include "WaterManager.hpp"
#include "WaterSurface.hpp"
#include "Terrain.hpp"

// ==================================================
// Draw all water surfaces
// ==================================================
void WaterManager::draw()
{
    for (auto* w : waters)
    {
        if (!w) continue;
        w->draw();
    }
}

// ==================================================
// 圓形水坑（小水窪）
// ==================================================
void WaterManager::addWaterPuddle(
    Terrain* terrain,
    float cx, float cz,
    float radius,
    float waterOffset
)
{
    if (!terrain) return;

    auto* w = new WaterSurface();

    w->origin     = glm::vec3(cx - radius, 0.0f, cz - radius);
    w->sizeX      = radius * 2.0f;
    w->sizeZ      = radius * 2.0f;
    w->offset     = waterOffset;
    w->resolution = 50;

    w->generateMesh(terrain);
    add(w);
}

// ==================================================
// 裂縫中的水
// ==================================================
void WaterManager::addCrevasseWater(
    Terrain* terrain,
    float cx, float cz,
    float width, float depth
)
{
    if (!terrain) return;

    auto* w = new WaterSurface();

    w->origin = glm::vec3(cx - width, 0.0f, cz - width);
    w->sizeX  = width * 2.0f;
    w->sizeZ  = width * 2.0f;

    // 水比裂縫底稍微高一點
    w->offset     = -depth + 0.1f;
    w->resolution = 70;

    w->generateMesh(terrain);
    add(w);
}

// ==================================================
// 河流：沿著 polyline 生成一串水面
// ==================================================
void WaterManager::addRiver(
    Terrain* terrain,
    const std::vector<glm::vec2>& points,
    float width,
    float waterOffset
)
{
    if (!terrain) return;
    if (points.size() < 2) return;

    // ✅ 間距要小於水面寬度，才不會有洞
    // 0.6f ~ 0.9f 可調：越小越密，水面越連續，但數量越多
    const float step = std::max(width * 0.7f, 1.0f);

    for (size_t i = 1; i < points.size(); ++i)
    {
        const glm::vec2 a = points[i - 1];
        const glm::vec2 b = points[i];

        const float segLen = glm::length(b - a);
        if (segLen < 1e-4f) continue;

        const int n = std::max(1, static_cast<int>(std::ceil(segLen / step)));

        for (int k = 0; k <= n; ++k)
        {
            float t = static_cast<float>(k) / static_cast<float>(n);
            glm::vec2 p = a + (b - a) * t;

            auto* w = new WaterSurface();
            w->origin = glm::vec3(p.x - width, 0.0f, p.y - width);
            w->sizeX  = width * 3.0f;
            w->sizeZ  = width * 3.0f;
            w->offset = waterOffset;

            // 多塊水面解析度不用太高，避免面數爆炸
            w->resolution = 30;

            w->generateMesh(terrain);
            add(w);
        }
    }
}

// ==================================================
// 圓湖
// ==================================================
void WaterManager::addLake(
    Terrain* terrain,
    float cx, float cz,
    float radius,
    float waterOffset
)
{
    if (!terrain) return;

    auto* w = new WaterSurface();

    w->origin = glm::vec3(cx - radius, 0.0f, cz - radius);
    w->sizeX  = radius * 2.0f;
    w->sizeZ  = radius * 2.0f;

    w->offset     = waterOffset;
    w->resolution = 100;

    // ★ 告訴 generateMesh 要做圓形（可在裡面用 radiusX=radiusY=radius 判斷）
    w->radius = radius;

    w->generateMesh(terrain);
    add(w);
}

// ==================================================
// 橢圓湖
// ==================================================
void WaterManager::addEllipseLake(
    Terrain* terrain,
    float cx, float cz,
    float radiusX, float radiusY,
    float waterOffset
)
{
    if (!terrain) return;

    auto* w = new WaterSurface();

    w->origin = glm::vec3(cx - radiusX, 0.0f, cz - radiusY);
    w->sizeX  = radiusX * 2.0f;
    w->sizeZ  = radiusY * 2.0f;
    w->offset = waterOffset;
    w->resolution = 120;

    // ★ 設定橢圓參數
    w->useEllipse = true;
    w->radiusX    = radiusX;
    w->radiusY    = radiusY;

    w->generateMesh(terrain);
    add(w);
}

// ==================================================
// 重新根據地形生成所有水面（若地形改變）
// ==================================================
void WaterManager::generateAll(const Terrain& terrain)
{
    for (auto* w : waters)
    {
        if (!w) continue;
        w->generateMesh(&terrain);
    }
}
