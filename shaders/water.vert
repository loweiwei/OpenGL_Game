#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTex;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTex;

uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// --------------------
// Wave height function
// --------------------
float waveHeight(vec3 p)
{
    float h = 0.0;
    h += sin(dot(p.xz, vec2( 1.0,  0.3)) * 0.12 + time * 1.3) * 0.25;
    h += sin(dot(p.xz, vec2(-0.6,  1.0)) * 0.18 + time * 0.9) * 0.15;
    h += sin(dot(p.xz, vec2( 0.7, -0.8)) * 0.35 + time * 2.0) * 0.08;
    h += sin(dot(p.xz, vec2(-1.0, -0.4)) * 0.55 + time * 3.2) * 0.04;
    h += sin(dot(p.xz, vec2( 0.2,  1.0)) * 0.90 + time * 4.0) * 0.02;
    return h;
}

void main()
{
    vec3 pos = aPos;

    // -----------------------------------------------------
    //  ★★★ 邊界不動（避免地形接縫裂開） ★★★
    // -----------------------------------------------------
    bool isEdge =
        (aTex.x < 0.001) || (aTex.x > 0.999) ||
        (aTex.y < 0.001) || (aTex.y > 0.999);

    float h = 0.0;

    if (!isEdge) {
        h = waveHeight(aPos);
    }

    pos.y += h;
    vPos  = pos;
    vTex  = aTex;

    // -----------------------------------------------------
    //  normal 使用 waveHeight 微分
    //  如果是邊界 → 直接使用 (0,1,0)
    // -----------------------------------------------------
    vec3 normal;

    if (!isEdge) {
        float eps = 0.1;
        float hL = waveHeight(aPos + vec3(eps, 0, 0));
        float hR = waveHeight(aPos - vec3(eps, 0, 0));
        float hD = waveHeight(aPos + vec3(0, 0, eps));
        float hU = waveHeight(aPos - vec3(0, 0, eps));
        normal = normalize(vec3(hL - hR, 2.0 * eps, hD - hU));
    } else {
        normal = vec3(0, 1, 0);
    }

    vNormal = normal;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}
