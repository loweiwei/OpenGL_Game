#include "Terrain.hpp"
#include "utils/PerlinNoise.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
static PerlinNoise gNoise(12345);

// 弧度 (0~1 → sin 曲線)
static float arcFade(float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return sin(t * 1.570796f); // sin(pi/2 * t)
}

Terrain::Terrain(int gx, int gz, float s, float maxH)
    : gridX(gx), gridZ(gz), size(s), maxHeight(maxH)
{
    // 創建空白 mask
    crevasseMask.resize(VC*VC, 0.0f);
    slopeMask.resize(VC*VC, 0.0f);
    flatMask.resize(VC*VC, 0.0f);
    flatHeightMap.resize(VC*VC, 0.0f);
    ridgeMask.resize(VC * VC, 0.0f);
    ridgeHeightMap.resize(VC * VC, 0.0f);


    waterNorth.resize(VC, 0);
    waterSouth.resize(VC, 0);
    waterWest.resize(VC, 0);
    waterEast.resize(VC, 0);
}

void Terrain::generateTerrain()
{
    const float TILE  = size / (VC - 1);
    const float BLEND = 20.0f;

    heightMap.resize(VC * VC);

    std::vector<float> vertices;
    vertices.reserve(VC * VC * 8);

    std::vector<unsigned int> indices;

    for (int z = 0; z < VC; z++)
    {
        for (int x = 0; x < VC; x++)
        {
            // ---------------------------------------------------
            // ★ 地形的 XZ 也要跟水一樣偏移 → 完全貼合
            // ---------------------------------------------------
            float worldX = gridX * size + x * TILE;
            float worldZ = gridZ * size + z * TILE;

            float offset = 0.0f;

            if (z == 0 && !waterNorth.empty())          offset = waterNorth[x];
            else if (z == VC - 1 && !waterSouth.empty()) offset = waterSouth[x];
            else if (x == 0 && !waterWest.empty())       offset = waterWest[z];
            else if (x == VC - 1 && !waterEast.empty())  offset = waterEast[z];

            // ★ XZ 偏移套用：與水一致
            if (z == 0 || z == VC - 1)
                worldZ += offset;
            if (x == 0 || x == VC - 1)
                worldX += offset;

            // ---------------------------------------------------
            // 原始地形（Perlin）
            // ---------------------------------------------------
            float h = gNoise.noise(worldX * 0.1f, worldZ * 0.1f) * maxHeight;

            // ---- 裂谷 ----
            if (crevasseMask[z*VC + x] > 0.0f)
            {
                h -= crevasseMask[z*VC + x] * crevasseDepth;
            }

            // ---- 斜坡 ----
            if (slopeStrength != 0.0f)
            {
                h += slopeMask[z*VC + x] * slopeStrength ;
            }

            // ----平地 ----
            if (flatMask[z * VC + x] > 0.0f)
            {
                float t = flatMask[z * VC + x];
                h = h * (1.0f - t) + flatHeightMap[z * VC + x] * t;
            }
            // ---- 山脊（彎曲山脈） ----
            if (ridgeMask[z * VC + x] > 0.0f)
            {
                float t = ridgeMask[z * VC + x];
                h += ridgeHeightMap[z * VC + x] * t;
            }

            // ---------------------------------------------------
            // 高度弧度 (不影響 XZ，只做自然過渡)
            // ---------------------------------------------------
            float distN = (float)z;
            float distS = (float)(VC - 1 - z);
            float distW = (float)x;
            float distE = (float)(VC - 1 - x);

            float fadeN = arcFade(distN / BLEND);
            float fadeS = arcFade(distS / BLEND);
            float fadeW = arcFade(distW / BLEND);
            float fadeE = arcFade(distE / BLEND);

            if (z == 0 && !waterNorth.empty())
                h = h * fadeN;  // 高度往 0 靠

            if (z == VC - 1 && !waterSouth.empty())
                h = h * fadeS;

            if (x == 0 && !waterWest.empty())
                h = h * fadeW;

            if (x == VC - 1 && !waterEast.empty())
                h = h * fadeE;

            // ---------------------------------------------------
            // 儲存高度
            // ---------------------------------------------------
            heightMap[z*VC + x] = h;

            // ---------------------------------------------------
            // 頂點
            // ---------------------------------------------------
            vertices.push_back(worldX);
            vertices.push_back(h);
            vertices.push_back(worldZ);

            vertices.push_back(0);
            vertices.push_back(1);
            vertices.push_back(0);

            float uvScale = 0.09f;    // 可調
            vertices.push_back(x * uvScale);
            vertices.push_back(z * uvScale);

        }
    }

    // ---------------------------------------------------
    // 索引
    // ---------------------------------------------------
    for (int z = 0; z < VC - 1; z++)
    for (int x = 0; x < VC - 1; x++)
    {
        int tl = z * VC + x;
        int tr = tl + 1;
        int bl = (z + 1) * VC + x;
        int br = bl + 1;

        indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
        indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
    }

    indexCount = indices.size();

    if (!vao) glGenVertexArrays(1, &vao);
    if (!vbo) glGenBuffers(1, &vbo);
    if (!ebo) glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size()*sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float),
                          (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float),
                          (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}
static float pointSegmentDistance(
    const glm::vec2& p,
    const glm::vec2& a,
    const glm::vec2& b,
    float& outT   // [0,1] 線段內比例
){
    glm::vec2 ab = b - a;
    float abLen2 = glm::dot(ab, ab);
    if (abLen2 < 1e-6f) {
        outT = 0.0f;
        return glm::length(p - a);
    }

    outT = glm::dot(p - a, ab) / abLen2;
    outT = std::clamp(outT, 0.0f, 1.0f);

    glm::vec2 proj = a + outT * ab;
    return glm::length(p - proj);
}
//====== 裂谷 addCrevasse()
void Terrain::addCrevasse(
    float cx, float cz,      // 中心位置（tile local space）
    float halfWidth,         // 橫向半徑
    float halfLength,        // 縱向半徑
    float depth,             // 裂縫深度
    float angleDeg           // 裂縫旋轉角度（度數）
)
{
    crevasseDepth = depth;

    float angle = glm::radians(angleDeg);
    float cosA = cos(angle);
    float sinA = sin(angle);

    for (int z = 0; z < VC; z++)
    for (int x = 0; x < VC; x++)
    {
        float fx = (float)x / (VC-1) * size;
        float fz = (float)z / (VC-1) * size;

        // 相對中心座標
        float dx = fx - cx;
        float dz = fz - cz;

        // 旋轉到裂縫方向
        float rx =  dx * cosA + dz * sinA;
        float rz = -dx * sinA + dz * cosA;

        // 橢圓距離（0~1）
        float nx = rx / halfWidth;
        float nz = rz / halfLength;
        float dist = sqrt(nx*nx + nz*nz);

        if (dist <= 1.0f)
        {
            // 平滑過渡
            float t = 1.0f - dist;
            t = t * t * (3 - 2 * t); // smoothstep

            crevasseMask[z*VC + x] = std::max(crevasseMask[z*VC + x], t);
        }
    }
}
//====== 彎曲裂谷 addCurvedCrevasse()
void Terrain::addCurvedCrevasse(
    const std::vector<glm::vec2>& points,
    float halfWidth,
    float depth
)
{
    if (points.size() < 2) return;

    // --------------------------------------------------
    // 1️⃣ 計算整條裂縫的累積長度
    // --------------------------------------------------
    std::vector<float> segStartLen(points.size(), 0.0f);
    float totalLen = 0.0f;

    for (size_t i = 1; i < points.size(); ++i)
    {
        totalLen += glm::length(points[i] - points[i - 1]);
        segStartLen[i] = totalLen;
    }

    if (totalLen < 1e-4f) return;

    // --------------------------------------------------
    // 2️⃣ 對每個地形點，找「最近線段」
    // --------------------------------------------------
    for (int z = 0; z < VC; ++z)
    for (int x = 0; x < VC; ++x)
    {
        float fx = (float)x / (VC - 1) * size;
        float fz = (float)z / (VC - 1) * size;
        glm::vec2 p(fx, fz);

        float minDist = 1e9f;
        float curvePos = 0.0f; // 在整條裂縫上的位置

        for (size_t i = 1; i < points.size(); ++i)
        {
            float t;
            float d = pointSegmentDistance(
                p, points[i - 1], points[i], t
            );

            if (d < minDist)
            {
                minDist = d;
                float segLen = glm::length(points[i] - points[i - 1]);
                curvePos = segStartLen[i - 1] + t * segLen;
            }
        }

        if (minDist >= halfWidth) continue;

        // --------------------------------------------------
        // 3️⃣ 橫向 smoothstep（寬度）
        // --------------------------------------------------
        float s = 1.0f - (minDist / halfWidth);
        s = s * s * (3.0f - 2.0f * s);

        // --------------------------------------------------
        // 4️⃣ 全曲線 length fade（只做一次）
        // --------------------------------------------------
        float u = curvePos / totalLen; // 0~1
        float lenFade = 1.0f - fabs(u * 2.0f - 1.0f);
        lenFade = lenFade * lenFade;

        s *= lenFade;

        int idx = z * VC + x;
        crevasseMask[idx] = std::max(crevasseMask[idx], s);
    }

    crevasseDepth = depth;
}

//====== 斜坡 addSlope()
void Terrain::addSlope(
    float dirX, float dirZ,
    float strength,
    float centerOffsetX,   // 坡中心（相對 tile 中心）
    float centerOffsetZ,
    float radius,          // 影響範圍（0 = 全 tile）
    float smoothness       // 平滑係數
)
{
    //std::cout << "[DEBUG] addSlope called1! strength=" << strength << "\n";
    this->slopeStrength = strength;

    // 正規化方向
    float len = sqrt(dirX * dirX + dirZ * dirZ);
    if (len < 0.001f) { dirX = 1; dirZ = 0; }
    else { dirX /= len; dirZ /= len; }

    // 世界中心
    float cx = gridX * size + size * 0.5f + centerOffsetX;
    float cz = gridZ * size + size * 0.5f + centerOffsetZ;

    float sum = 0.0f;

    for (int z = 0; z < VC; z++)
    for (int x = 0; x < VC; x++)
    {
        float wx = gridX * size + (float)x / (VC - 1) * size;
        float wz = gridZ * size + (float)z / (VC - 1) * size;

        float dx = wx - cx;
        float dz = wz - cz;

        float dist = sqrt(dx*dx + dz*dz);

        // 初始坡度（線性）
        float t = dx * dirX + dz * dirZ;

        // 半徑控制
        if (radius > 0.0f)
        {
            float r = dist / radius;
            if (r >= 1.0f)
            {
                slopeMask[z*VC + x] += 0;
                continue;
            }

            // smooth fade: r 越大 → 衰減越多
            float fade = pow(1.0f - r, smoothness);
            t *= fade;
        }
        //std::cout << "[DEBUG] addSlope called2! strength=" << strength << "\n";

        slopeMask[z*VC + x] = t * strength;
        sum += slopeMask[z*VC + x];
    }

    // 平均歸零，不會抬高整地
    float mean = sum / (VC * VC);
    for (int i = 0; i < VC*VC; i++)
        slopeMask[i] -= mean;
}

//====== 平台 addFlatArea()
void Terrain::addFlatArea(float cx, float cz,
                          float radius,
                          float flatHeight,
                          float smoothness)
{
    if (radius <= 0.0f) return;

    for (int z = 0; z < VC; z++)
    for (int x = 0; x < VC; x++)
    {
        float fx = (float)x / (VC - 1) * size;
        float fz = (float)z / (VC - 1) * size;

        float dx = fx - cx;
        float dz = fz - cz;
        float dist = sqrt(dx*dx + dz*dz);

        if (dist > radius) continue;

        float t = 1.0f - (dist / radius);
        t = pow(t, smoothness);   // 0~1

        int idx = z * VC + x;

        // 記錄這格平坦程度（mask）
        flatMask[idx] = std::max(flatMask[idx], t);

        // 記錄希望變成的高度（flatHeight）
        flatHeightMap[idx] = flatHeight;
    }
}

void Terrain::addRidge(
    const std::vector<glm::vec2>& points,
    float halfWidth,
    float height)
{
    if (points.size() < 2) return;

    // ----------------------------------------
    // 1️⃣ 累積整條 ridge 長度
    // ----------------------------------------
    std::vector<float> segStartLen(points.size(), 0.0f);
    float totalLen = 0.0f;

    for (size_t i = 1; i < points.size(); ++i)
    {
        totalLen += glm::length(points[i] - points[i - 1]);
        segStartLen[i] = totalLen;
    }

    if (totalLen < 1e-4f) return;

    // ----------------------------------------
    // 2️⃣ 對每個地形點找最近線段
    // ----------------------------------------
    for (int z = 0; z < VC; ++z)
    for (int x = 0; x < VC; ++x)
    {
        float fx = (float)x / (VC - 1) * size;
        float fz = (float)z / (VC - 1) * size;
        glm::vec2 p(fx, fz);

        float minDist = 1e9f;
        float curvePos = 0.0f;

        for (size_t i = 1; i < points.size(); ++i)
        {
            float t;
            float d = pointSegmentDistance(
                p, points[i - 1], points[i], t
            );

            if (d < minDist)
            {
                minDist = d;
                float segLen = glm::length(points[i] - points[i - 1]);
                curvePos = segStartLen[i - 1] + t * segLen;
            }
        }

        if (minDist >= halfWidth) continue;

        // ----------------------------------------
        // 3️⃣ 橫向 smoothstep（寬度）
        // ----------------------------------------
        float s = 1.0f - (minDist / halfWidth);
        s = s * s * (3.0f - 2.0f * s);

        // ----------------------------------------
        // 4️⃣ 全曲線頭尾 fade
        // ----------------------------------------
        float u = curvePos / totalLen; // 0~1
        float t = 1.0f - fabs(u * 2.0f - 1.0f);
        float lenFade = pow(t, 0.6f);
        s *= lenFade;


        int idx = z * VC + x;
        ridgeMask[idx] = std::max(ridgeMask[idx], s);
        ridgeHeightMap[idx] = height;
    }
}

void Terrain::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

float Terrain::getHeight(float worldX, float worldZ) const
{
    float localX = worldX - gridX * size;
    float localZ = worldZ - gridZ * size;

    const float TILE = size / (VC - 1);

    float fx = localX / TILE;
    float fz = localZ / TILE;

    int ix = std::clamp((int)fx, 0, VC - 2);
    int iz = std::clamp((int)fz, 0, VC - 2);

    float tx = fx - ix;
    float tz = fz - iz;

    auto H = [&](int x, int z){
        return heightMap[z*VC + x];
    };

    float h0 = H(ix, iz);
    float h1 = H(ix+1, iz);
    float h2 = H(ix, iz+1);
    float h3 = H(ix+1, iz+1);

    float A = h0*(1-tx) + h1*tx;
    float B = h2*(1-tx) + h3*tx;

    return A*(1-tz) + B*tz;
}

void Terrain::setWaterNorth(const std::vector<float>& s){ waterNorth = s; }
void Terrain::setWaterSouth(const std::vector<float>& s){ waterSouth = s; }
void Terrain::setWaterWest (const std::vector<float>& s){ waterWest  = s; }
void Terrain::setWaterEast (const std::vector<float>& s){ waterEast  = s; }
