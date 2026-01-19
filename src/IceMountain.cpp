#include "IceMountain.hpp"
#include <glad/glad.h>
#include <glm/gtc/noise.hpp>
#include <cmath>
#include <iostream>

// ======================================================
// Constructor
// ======================================================
IceMountain::IceMountain(glm::vec3 pos, float r, float h,
                         Direction door1, Direction door2)
    : position(pos), radius(r), height(h),
      frontDoor(door1), backDoor(door2)
{
}

// ======================================================
// Add Vertex Helper
// ======================================================
void IceMountain::addVertex(std::vector<float>& v,
                            glm::vec3 pos,
                            glm::vec3 normal,
                            glm::vec2 uv)
{
    v.push_back(pos.x);
    v.push_back(pos.y);
    v.push_back(pos.z);

    v.push_back(normal.x);
    v.push_back(normal.y);
    v.push_back(normal.z);

    v.push_back(uv.x);
    v.push_back(uv.y);
}

// ======================================================
// Smoothstep
// ======================================================
static float smoothstep(float edge0, float edge1, float x)
{
    float t = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// ======================================================
// Compute Normals
// ======================================================
void IceMountain::computeNormals(std::vector<float>& vertices,
                                 const std::vector<unsigned int>& indices)
{
    // clear
    for (int i = 0; i < vertices.size(); i += 8)
    {
        vertices[i+3] = 0;
        vertices[i+4] = 0;
        vertices[i+5] = 0;
    }

    // accumulate faces
    for (int i = 0; i < indices.size(); i += 3)
    {
        unsigned int i0 = indices[i] * 8;
        unsigned int i1 = indices[i+1] * 8;
        unsigned int i2 = indices[i+2] * 8;

        glm::vec3 v0(vertices[i0],     vertices[i0+1],     vertices[i0+2]);
        glm::vec3 v1(vertices[i1],     vertices[i1+1],     vertices[i1+2]);
        glm::vec3 v2(vertices[i2],     vertices[i2+1],     vertices[i2+2]);

        glm::vec3 N = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        vertices[i0+3] += N.x; vertices[i0+4] += N.y; vertices[i0+5] += N.z;
        vertices[i1+3] += N.x; vertices[i1+4] += N.y; vertices[i1+5] += N.z;
        vertices[i2+3] += N.x; vertices[i2+4] += N.y; vertices[i2+5] += N.z;
    }

    // normalize
    for (int i = 0; i < vertices.size(); i += 8)
    {
        glm::vec3 N(vertices[i+3], vertices[i+4], vertices[i+5]);
        N = glm::normalize(N);

        vertices[i+3] = N.x;
        vertices[i+4] = N.y;
        vertices[i+5] = N.z;
    }

    for (int i = 0; i < vertices.size(); i += 8)
    {
        glm::vec3 N(vertices[i+3], vertices[i+4], vertices[i+5]);
        glm::vec3 P(vertices[i+0], vertices[i+1], vertices[i+2]);

        /*if (i % (500*8) == 0)
        {
            printf("Pos(%.2f %.2f %.2f) Normal(%.2f %.2f %.2f)\n",
                P.x, P.y, P.z, N.x, N.y, N.z);
        }*/

    }

}

// ======================================================
// Main Iceberg Shape Generation
// ======================================================
void IceMountain::generateIceShape(std::vector<float>& vertices,
                                   std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();

    const int slices = 200;
    const int stacks = 150;

    float WIDTH  = radius;
    float TOP_H  = height * 0.8f;
    float BOT_H  = height * 0.5f;

    auto ridge = [&](float v){
        v = fabs(v);
        return 1.0f - v;
    };

    // ----------------------------
    // Top surface
    // ----------------------------
    for (int y = 0; y <= stacks; y++)
    {
        float vy = float(y) / stacks;

        for (int x = 0; x <= slices; x++)
        {
            float vx = float(x) / slices;

            float px = vx * 2.f - 1.f;
            float pz = vy * 2.f - 1.f;

            glm::vec2 p(px, pz);

            glm::vec2 peakA(-0.4f, -0.2f);
            glm::vec2 peakB( 0.3f, 0.3f);
            glm::vec2 peakC( 0.1f,-0.6f);

            float dA = glm::length(p - peakA);
            float dB = glm::length(p - peakB);
            float dC = glm::length(p - peakC);

            float hA = expf(-dA * dA * 7.0f);
            float hB = expf(-dB * dB * 5.0f);
            float hC = expf(-dC * dC * 4.0f);

            float peaks = hA * 1.7f + hB * 1.3f + hC * 0.9f;
            peaks = sqrt(peaks);

            float rx = glm::perlin(glm::vec3(px * 4, pz * 4, peaks * 2));
            float rz = glm::perlin(glm::vec3(px * 7, pz * 7, peaks * 3));
            float ridgeNoise = ridge(rx)*0.2f + ridge(rz)*0.2f;

            float detail = glm::perlin(glm::vec3(px * 6, pz * 6, peaks)) * 0.1f;

            float finalPeak = peaks * 1.2f + ridgeNoise * 0.3f + detail;

            glm::vec3 pos(px * WIDTH, finalPeak * TOP_H, pz * WIDTH);

            pos.y *= (1.0f + peaks * 1.8f);

            float uvScale = 2.0f;   // 可調 2~8
            glm::vec3 fakeN(0,1,0);
            addVertex(vertices, pos, fakeN, glm::vec2(vx * uvScale, vy * uvScale));


        }
    }

    // ----------------------------
    // Flat bottom
    // ----------------------------
    int bottomStart = vertices.size() / 8;

    for (int y = 0; y <= stacks; y++)
    {
        float vy = float(y) / stacks;

        for (int x = 0; x <= slices; x++)
        {
            float vx = float(x) / slices;

            float px = vx * 2.f - 1.f;
            float pz = vy * 2.f - 1.f;

            glm::vec3 pos(px * WIDTH, -BOT_H, pz * WIDTH);

            addVertex(vertices, pos, glm::vec3(0,-1,0), glm::vec2(vx,vy));
        }
    }

    // ----------------------------
    // Build triangles
    // ----------------------------
    int row = slices + 1;

    auto build = [&](int start)
    {
        for (int y = 0; y < stacks; y++)
        for (int x = 0; x < slices; x++)
        {
            int tl = start + y*row + x;
            int tr = tl + 1;
            int bl = start + (y+1)*row + x;
            int br = bl + 1;

            indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
            indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
        }
    };

    build(0);
    build(bottomStart);

    // recalc normals
    computeNormals(vertices, indices);

    // ----------------------------
    // sink iceberg
    // ----------------------------
    float minY = 99999;
    for (int i = 0; i < vertices.size(); i += 8)
        minY = std::min(minY, vertices[i+1]);

    float sinkValue = 17.0f;
    for (int i = 0; i < vertices.size(); i += 8)
        vertices[i+1] -= (minY + sinkValue);
}

// ======================================================
// TUNNEL / DOOR CARVING
// ======================================================
void IceMountain::carveDoor(std::vector<float>& vertices,
                            Direction dir)
{
    float tunnelRadius = radius * 0.35f;
    float tunnelY = height * 0.25f;

    // decide axis direction
    glm::vec3 axisDir(0);

    if (dir == EAST)  axisDir = glm::vec3(1,0,0);
    if (dir == WEST)  axisDir = glm::vec3(-1,0,0);
    if (dir == NORTH) axisDir = glm::vec3(0,0,-1);
    if (dir == SOUTH) axisDir = glm::vec3(0,0, 1);

    axisDir = glm::normalize(axisDir);

    glm::vec3 A = glm::vec3(0,tunnelY,0) - axisDir * radius * 1.1f;
    glm::vec3 B = glm::vec3(0,tunnelY,0) + axisDir * radius * 1.1f;

    glm::vec3 AB = B - A;
    float lenAB = glm::length(AB);
    glm::vec3 dirAB = glm::normalize(AB);

    float fadeRange = tunnelRadius * 1.5f;

    for (int i = 0; i < vertices.size(); i += 8)
    {
        float& x = vertices[i+0];
        float& y = vertices[i+1];
        float& z = vertices[i+2];

        glm::vec3 P(x,y,z);

        // projection
        float t = glm::dot(P - A, dirAB);
        if (t < 0 || t > lenAB)
            continue;

        glm::vec3 C = A + t * dirAB;
        float distAxis = glm::length(P - C);

        float distEnd = std::min(
            glm::length(P - A),
            glm::length(P - B)
        );

        float fade = glm::clamp(distEnd / fadeRange, 0.0f, 1.0f);

        glm::vec3 cyl = C + (P - C) * (tunnelRadius / (distAxis + 0.00001f));
        glm::vec3 result = glm::mix(P, cyl, 1.0f - fade);

        x = result.x;
        y = result.y;
        z = result.z;
    }
}

// ======================================================
// Upload Mesh to OpenGL
// ======================================================
void IceMountain::generateMesh()
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    generateIceShape(vertices, indices);

    // apply tunnel
    //carveDoor(vertices, frontDoor);
    //carveDoor(vertices, backDoor);

    indexCount = indices.size();

    if (!vao) glGenVertexArrays(1, &vao);
    if (!vbo) glGenBuffers(1, &vbo);
    if (!ebo) glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float)*vertices.size(),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(unsigned int)*indices.size(),
                 indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,false,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,3,GL_FLOAT,false,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,2,GL_FLOAT,false,8*sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// ======================================================
// Draw
// ======================================================
void IceMountain::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}
