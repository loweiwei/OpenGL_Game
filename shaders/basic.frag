//算海報貼在哪裡
//TexCoord = 海報上某個點
//texture1 = 那張海報
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
