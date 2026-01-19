#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform float minHeight;
uniform float maxHeight;

// ==================================================
// Utility: very soft noise
// ==================================================
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main()
{
    // ==================================================
    // 1️⃣ Base texture
    // ==================================================
    vec3 base = texture(texture1, TexCoord).rgb;

    // ==================================================
    // 2️⃣ Height normalize (0~1)
    // ==================================================
    float h = clamp(
        (FragPos.y - minHeight) / (maxHeight - minHeight),
        0.0, 1.0
    );

    // ==================================================
    // 3️⃣ Height-based color (LOW CONTRAST!)
    // ==================================================
    vec3 soil = vec3(0.74, 0.72, 0.70);
    vec3 rock = vec3(0.82, 0.82, 0.83);
    vec3 snow = vec3(0.88, 0.91, 0.95);   // ❗ 絕對不要純白

    vec3 heightColor = mix(soil, rock, smoothstep(0.12, 0.60, h));
    heightColor = mix(heightColor, snow, smoothstep(0.65, 0.92, h));

    vec3 color = base * heightColor;

    // ==================================================
    // 4️⃣ Normal & slope
    // ==================================================
    vec3 N = normalize(Normal);
    float slope = 1.0 - clamp(dot(N, vec3(0,1,0)), 0.0, 1.0);

    // ==================================================
    // 5️⃣ Fake Sun Light (MOST IMPORTANT)
    // ==================================================
    vec3 sunDir   = normalize(vec3(0.4, 0.9, 0.2)); // 從右上
    vec3 sunColor = vec3(0.95, 0.97, 1.00);         // 冷日光

    float NdotL = clamp(dot(N, sunDir), 0.0, 1.0);

    // 不要太硬，留環境光
    float light = 0.45 + 0.55 * NdotL;

    color *= sunColor * light;

    // ==================================================
    // 6️⃣ Cold slope shadow (NOT BLACK!)
    // ==================================================
    float slopeFactor = smoothstep(0.45, 0.90, slope);
    vec3 coldShadow = color * vec3(0.86, 0.90, 0.97);
    color = mix(color, coldShadow, slopeFactor);

    // ==================================================
    // 7️⃣ Snow accumulation (flat > steep)
    // ==================================================
    float snowMask =
        smoothstep(0.70, 0.93, h) *
        (1.0 - smoothstep(0.30, 0.75, slope));

    color = mix(color, snow, snowMask * 0.45);

    // ==================================================
    // 8️⃣ Sky ambient tint (PHOTO FEEL)
    // ==================================================
    vec3 skyTint = vec3(0.78, 0.85, 0.95);
    color = mix(color, color * skyTint, 0.25);

    // ==================================================
    // 9️⃣ Micro noise (break uniformity)
    // ==================================================
    float n = hash(FragPos.xz * 0.10);
    color = mix(color, color * 0.97, n);   // 最多只暗 3%

    // ==================================================
    // 🔟 Distance fog (AIR PERSPECTIVE)
    // ==================================================
    float dist = length(FragPos.xz);
    float fog = smoothstep(350.0, 900.0, dist);

    vec3 fogColor = vec3(0.82, 0.88, 0.95);
    color = mix(color, fogColor, fog * 0.6);

    // ==================================================
    // 1️⃣1️⃣ Tone compression (camera response)
    // ==================================================
    color = color * 0.9 + 0.05;

    // ==================================================
    // 1️⃣2️⃣ Very subtle Fresnel (air glow)
    // ==================================================
    float fresnel = pow(1.0 - dot(N, vec3(0,1,0)), 4.0);
    color += fresnel * 0.015;

    FragColor = vec4(color, 1.0);
}
