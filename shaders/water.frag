#version 330 core
out vec4 FragColor;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTex;

uniform vec3 lightPos;
uniform vec3 cameraPos;


void main()
{
    // ========== Base 水色 ==========
    vec3 deep    = vec3(0.20, 0.70, 1.00);   // 深水 → 明顯亮藍
    vec3 shallow = vec3(0.90, 1.00, 1.00);   // 淺水 → 幾乎白藍、很亮



    float depth = clamp((vPos.y + 0.4) / 3.0*2, 0.0, 1.0);
    vec3 waterColor = mix(deep, shallow, depth);

    // ========== 光照 ==========
    vec3 N = normalize(vNormal);
    vec3 L = normalize(lightPos - vPos);

    float diff = max(dot(N, L), 0.0);

    // 基本光
    vec3 color = waterColor * (0.3 + 0.7 * diff);

    // ========== 波光反射 Sparkle ==========
    vec3 V = normalize(cameraPos - vPos);
    vec3 H = normalize(V + L);

    float sparkle = pow(max(dot(N, H), 0.0), 80.0);
    color += vec3(1.0, 1.0, 1.0) * sparkle * 0.4;

    FragColor = vec4(color, 1.0);
}


