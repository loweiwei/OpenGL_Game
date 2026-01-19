#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class ObjMesh {
public:
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };
    
    std::vector<Vertex> vertices;   // ⭐ 原本 private → 現在 public
    std::vector<unsigned int> indices;

    ObjMesh(const std::string& path);
    void draw() const;

private:
    

    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int textureID = 0;   // ⭐ 新增：材質貼圖 ID
    int indexCount = 0;

    void load(const std::string& path);
    void loadTexture(const std::string& filename);
};
