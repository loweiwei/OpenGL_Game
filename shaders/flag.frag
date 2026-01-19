#version 330 core
out vec4 FragColor;

uniform vec3 flagColor; // 旗桿顏色

void main()
{
    FragColor = vec4(flagColor, 1.0);
}
