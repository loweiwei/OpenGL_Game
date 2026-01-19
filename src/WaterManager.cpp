#include "WaterManager.hpp"
#include "WaterSurface.hpp"
#include "Terrain.hpp"

void WaterManager::draw() {
    for (auto w : waters) {
        w->draw();
    }
}

//形狀：圓盤
void WaterManager::addWaterPuddle(Terrain* terrain,
                                  float cx, float cz,
                                  float radius,
                                  float waterOffset)
{
    WaterSurface* w = new WaterSurface();
    w->origin = glm::vec3(cx - radius, 0, cz - radius);
    w->sizeX  = radius*2;
    w->sizeZ  = radius*2;
    w->offset = waterOffset;
    w->resolution = 50;

    w->generateMesh(terrain);
    add(w);
}

void WaterManager::addCrevasseWater(Terrain* terrain,
                                    float cx, float cz,
                                    float width, float depth)
{
    WaterSurface* w = new WaterSurface();

    w->origin = glm::vec3(cx - width, 0, cz - width);
    w->sizeX = width*2;
    w->sizeZ = width*2;

    w->offset = -depth + 0.1f; // 水比裂縫底稍微高

    w->resolution = 70;

    w->generateMesh(terrain);
    add(w);
}

void WaterManager::addRiver(Terrain* terrain,
                            const std::vector<glm::vec2>& points,
                            float width,
                            float waterOffset)
{
    if (!terrain) return;
    if (points.size() < 2) return;

    // ✅ 關鍵：間距要小於水面寬度，才不會斷
    // 你可以調 0.6f~0.9f：越小越密、越不會漏，但水面數量會變多
    const float step = std::max(width * 0.7f, 1.0f);

    for (size_t i = 1; i < points.size(); ++i)
    {
        glm::vec2 a = points[i - 1];
        glm::vec2 b = points[i];

        float segLen = glm::length(b - a);
        if (segLen < 1e-4f) continue;

        int n = std::max(1, (int)std::ceil(segLen / step));

        for (int k = 0; k <= n; ++k)
        {
            float t = (float)k / (float)n;
            glm::vec2 p = a + (b - a) * t;

            WaterSurface* w = new WaterSurface();
            w->origin = glm::vec3(p.x - width, 0, p.y - width);
            w->sizeX  = width * 3.0f;
            w->sizeZ  = width * 3.0f;
            w->offset = waterOffset;

            // 多塊水面不用太高解析度，不然面數爆炸
            w->resolution = 30;

            w->generateMesh(terrain);
            add(w);
        }
    }
}

//圓湖
void WaterManager::addLake(Terrain* terrain,
                           float cx, float cz,
                           float radius,
                           float waterOffset)
{
    WaterSurface* w = new WaterSurface();

    w->origin = glm::vec3(cx - radius, 0, cz - radius);
    w->sizeX  = radius * 2;
    w->sizeZ  = radius * 2;

    w->offset = waterOffset;
    w->resolution = 100;

    w->radius = radius;   // ★告訴 generateMesh 要做圓形

    w->generateMesh(terrain);
    add(w);
}
//橢圓湖
void WaterManager::addEllipseLake(Terrain* terrain,
                                  float cx, float cz,
                                  float radiusX, float radiusY,
                                  float waterOffset)
{
    WaterSurface* w = new WaterSurface();

    w->origin = glm::vec3(cx - radiusX, 0, cz - radiusY);
    w->sizeX  = radiusX * 2;
    w->sizeZ  = radiusY * 2;
    w->offset = waterOffset;
    w->resolution = 120;

    // ★ 設定橢圓
    w->useEllipse = true;
    w->radiusX = radiusX;
    w->radiusY = radiusY;

    w->generateMesh(terrain);
    add(w);
}

void WaterManager::generateAll(const Terrain& terrain)
{
    for (auto w : waters)
    {
        if (!w) continue;
        w->generateMesh(&terrain);   // ★ 重新根據地形高度生成
    }
}
