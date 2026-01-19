#version 330 core
layout (location = 0) in vec3 aPos; 
layout (location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 vUV;
out float vHeight;    // ★ 地形高度傳給 fragment

void main()
{
    vUV = aUV;
    vHeight = aPos.y;  // ★ 地形高度
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
