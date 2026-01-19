#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D diffuseTex;
uniform vec3 color;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 tex = texture(diffuseTex, TexCoord).rgb;

    vec3 base = tex * color;
    vec3 ambient = 0.4 * base;
    vec3 diffuse = diff * base;

    FragColor = vec4(ambient + diffuse, 1.0);
}
