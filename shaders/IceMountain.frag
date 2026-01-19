#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D iceTex;
uniform sampler2D snowTex;

uniform float snowMinHeight;
uniform float snowMaxHeight;
uniform float snowSmoothness;

void main()
{
    vec3 N = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.4));

    // --- Ambient + Diffuse ---
    float ambient = 0.25;
    float diff = max(dot(N, lightDir), 0.0);
    float light = ambient + diff;

    // --- textures ---
    vec3 iceColor  = texture(iceTex, TexCoord).rgb;
    vec3 snowColor = texture(snowTex, TexCoord).rgb;

    float t = (FragPos.y - snowMinHeight) / (snowMaxHeight - snowMinHeight);
    t = smoothstep(0.0, snowSmoothness, t);
    t = clamp(t, 0.0, 1.0);

    vec3 finalColor = mix(iceColor, snowColor, t);

    FragColor = vec4(finalColor * light, 1.0);
}
