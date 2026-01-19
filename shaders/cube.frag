#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 cameraPos;
uniform vec3 lightPos;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(cameraPos - FragPos);

    // --- 基礎冰藍色 ---
    vec3 deepBlue  = vec3(0.0, 0.35, 0.7);
    vec3 lightBlue = vec3(0.6, 0.9, 1.0);
    float t = clamp(dot(N, vec3(0,1,0)) * 0.5 + 0.5, 0.0, 1.0);
    vec3 baseColor = mix(deepBlue, lightBlue, t);

    // --- Diffuse ---
    float diff = max(dot(N, L), 0.15);

    // --- Fresnel（冰的邊緣會亮）---
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);
    vec3 fresnelColor = vec3(0.6, 0.9, 1.0) * fresnel * 1.8;

    // --- Specular 高光 ---
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0);
    vec3 specColor = vec3(1.0) * spec * 0.5;

    vec3 finalColor = baseColor * diff + fresnelColor + specColor;

    FragColor = vec4(finalColor, 1.0);
}
