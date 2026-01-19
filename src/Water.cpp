#include "Water.hpp"
#include <cstdlib>

Water::Water(int gridX, int gridZ, float size)
    : gridX(gridX), gridZ(gridZ), size(size)
{
    srand(7777 + gridX * 123 + gridZ * 456);

    northShape.resize(VC);
    southShape.resize(VC);
    westShape.resize(VC);
    eastShape.resize(VC);

    // ======== 更自然的 fractal noise（明顯變化） =========
    auto noise = [&](float t)
    {
        float n =
            sin(t * 4.0f) * 0.8f +
            sin(t * 11.0f) * 0.4f +
            sin(t * 23.0f) * 0.2f;

        return n;
    };

    for (int i = 0; i < VC; i++)
    {
        float t = i * 0.1f;  // ★ 增加 frequency，讓湖岸真正動起來

        northShape[i] = noise(t + gridX * 2.3f) * 3.0f;  
        southShape[i] = noise(t + gridX * 2.3f + 10.0f) * 3.0f;
        westShape[i]  = noise(t + gridZ * 2.3f + 20.0f) * 3.0f;
        eastShape[i]  = noise(t + gridZ * 2.3f + 30.0f) * 3.0f;
    }

    // ======== 再平滑兩次（去鋸齒） =========
    auto smooth = [&](std::vector<float>& v)
    {
        std::vector<float> tmp = v;
        for (int i = 1; i < VC - 1; i++)
            v[i] = (tmp[i - 1] + tmp[i] + tmp[i + 1]) / 3.0f;

        tmp = v;
        for (int i = 1; i < VC - 1; i++)
            v[i] = (tmp[i - 1] + tmp[i] + tmp[i + 1]) / 3.0f;
    };

    smooth(northShape);
    smooth(southShape);
    smooth(westShape);
    smooth(eastShape);
}

void Water::generateMesh()
{
    const float TILE = size / (VC - 1);
    const float waterLevel = 0.0f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(VC * VC * 5);

    for (int z = 0; z < VC; z++)
    {
        for (int x = 0; x < VC; x++)
        {
            float worldX = gridX * size + x * TILE;
            float worldZ = gridZ * size + z * TILE;

            float offset = 0.0f;

            if (z == 0)             offset = northShape[x];
            else if (z == VC - 1)   offset = southShape[x];
            else if (x == 0)        offset = westShape[z];
            else if (x == VC - 1)   offset = eastShape[z];

            if (z == 0 || z == VC - 1) worldZ += offset;
            if (x == 0 || x == VC - 1) worldX += offset;

            vertices.push_back(worldX);
            vertices.push_back(waterLevel);
            vertices.push_back(worldZ);

            // ★★ 正確 UV (0~1)
            float u = (float)x / (VC - 1);
            float v = (float)z / (VC - 1);
            vertices.push_back(u);
            vertices.push_back(v);

        }
    }

    // indices
    for (int z = 0; z < VC - 1; z++)
    {
        for (int x = 0; x < VC - 1; x++)
        {
            int tl = z * VC + x;
            int tr = tl + 1;
            int bl = (z + 1) * VC + x;
            int br = bl + 1;

            indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
            indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
        }
    }

    indexCount = indices.size();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float),
                          (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Water::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}
