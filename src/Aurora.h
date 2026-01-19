#pragma once
#include "Shader.h"
#include "Camera.h"

class Aurora {
public:
    Aurora();
    void draw(Camera& camera,
          const glm::mat4& projection,
          float time);


private:
    unsigned int vao, vbo;
    Shader shader;
};
