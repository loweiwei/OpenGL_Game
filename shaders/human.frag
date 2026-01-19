#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 cameraPos;

// 顏色設定
uniform vec3 baseColor = vec3(1.0, 0.8, 0.6);  // 肌膚色
uniform vec3 lightPos;

void main()
{
    // normalize
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // basic diffuse
    float diff = max(dot(norm, lightDir), 0.3);

    // fake rim light (邊緣亮)
    float rim = 1.0 - max(dot(norm, normalize(cameraPos - FragPos)), 0.0);
    rim = pow(rim, 2.0);

    vec3 color = baseColor * diff + rim * 0.2;

    FragColor = vec4(color, 1.0);
}
