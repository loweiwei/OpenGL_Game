#include "WaterSurface.hpp"
#include "Terrain.hpp"
#include <glad/glad.h>
#include "iostream"

WaterSurface::WaterSurface()
    : offset(0.0f),
      sizeX(10.0f),
      sizeZ(10.0f),
      resolution(40),
      vao(0), vbo(0), ebo(0),
      indexCount(0),
      origin(0,0,0)
{}

void WaterSurface::generateMesh(const Terrain* terrain)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float cx = origin.x + sizeX * 0.5f;
    float cz = origin.z + sizeZ * 0.5f;

    float stepX = sizeX / (resolution - 1);
    float stepZ = sizeZ / (resolution - 1);

    // 暫存所有點
    struct V { float x,y,z,u,v; bool valid; };
    std::vector<V> temp(resolution * resolution);

    // === 建立頂點 ===
    for (int z = 0; z < resolution; z++)
    for (int x = 0; x < resolution; x++)
    {
        float wx = origin.x + x * stepX;
        float wz = origin.z + z * stepZ;

        float u = (float)x / (resolution - 1);
        float v = (float)z / (resolution - 1);

        float h = offset;

        bool valid = true;

        if (radiusX > 0 && radiusY > 0)  // ★ 支援橢圓
        {
            float dx = (wx - cx) / radiusX;
            float dz = (wz - cz) / radiusY;
            if (dx*dx + dz*dz > 1.0f)
                valid = false;
        }

        int id = z * resolution + x;
        temp[id] = { wx, h, wz, u, v, valid };
    }

    // === compact vertex ===
    std::vector<int> newIndex(resolution * resolution, -1);
    int counter = 0;

    for (int i = 0; i < temp.size(); i++)
    {
        if (!temp[i].valid) continue;

        newIndex[i] = counter++;

        vertices.push_back(temp[i].x);
        vertices.push_back(temp[i].y);
        vertices.push_back(temp[i].z);

        vertices.push_back(0);
        vertices.push_back(1);
        vertices.push_back(0);

        vertices.push_back(temp[i].u);
        vertices.push_back(temp[i].v);
    }

    // === 建立 index ===
    for (int z = 0; z < resolution - 1; z++)
    for (int x = 0; x < resolution - 1; x++)
    {
        int tl = z * resolution + x;
        int tr = tl + 1;
        int bl = tl + resolution;
        int br = bl + 1;

        if (newIndex[tl] < 0 || newIndex[tr] < 0 ||
            newIndex[bl] < 0 || newIndex[br] < 0)
            continue;

        indices.push_back(newIndex[tl]);
        indices.push_back(newIndex[bl]);
        indices.push_back(newIndex[tr]);

        indices.push_back(newIndex[tr]);
        indices.push_back(newIndex[bl]);
        indices.push_back(newIndex[br]);
    }

    indexCount = indices.size();

    // === Upload to GPU ===
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



void WaterSurface::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}
