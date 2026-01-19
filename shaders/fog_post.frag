#version 330 core
out vec4 FragColor;
in vec2 vUV;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
// inverse(projection * view)
uniform mat4 invViewProj;
uniform vec3 cameraPos;
// 地圖範圍（XZ）
uniform vec2 mapMinXZ;
uniform vec2 mapMaxXZ;
// 外圍霧帶厚度（往內）
uniform float beltWidth;
// 霧色 / 最大霧量（上限）
uniform vec3  fogColor;
uniform float fogMax;
// 噪聲
uniform float time;
uniform float noiseScale;
uniform float noiseAmp;
// 體積霧參數
uniform float density;
uniform float fogBaseY;
uniform float fogHeight;
uniform int   fogSteps;

// 新增：空氣要 march 多遠（建議=far plane，例如 600）
uniform float fogMaxDist = 600;

// ---------------- noise ----------------
float hash21(vec2 p){
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}
float vnoise(vec2 p){
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash21(i);
    float b = hash21(i + vec2(1,0));
    float c = hash21(i + vec2(0,1));
    float d = hash21(i + vec2(1,1));
    vec2 u = f*f*(3.0-2.0*f);
    return mix(a,b,u.x) + (c-a)*u.y*(1.0-u.x) + (d-b)*u.x*u.y;
}
float fbm(vec2 p){
    float v = 0.0;
    float a = 0.5;
    for(int k=0;k<4;k++){
        v += a * vnoise(p);
        p *= 2.02;
        a *= 0.5;
    }
    return v; // ~0..1
}

vec3 reconstructWorldPos(vec2 uv, float depth01)
{
    vec2 ndcXY = uv * 2.0 - 1.0;
    float ndcZ = depth01 * 2.0 - 1.0;

    vec4 clip  = vec4(ndcXY, ndcZ, 1.0);
    vec4 world = invViewProj * clip;
    world.xyz /= world.w;
    return world.xyz;
}

// 地圖邊界霧權重（0..1）
float edgeWeight(vec3 P)
{
    float dx = min(P.x - mapMinXZ.x, mapMaxXZ.x - P.x);
    float dz = min(P.z - mapMinXZ.y, mapMaxXZ.y - P.z);
    float dEdge = min(dx, dz); // 0=邊界

    bool outside =
        (P.x < mapMinXZ.x) || (P.x > mapMaxXZ.x) ||
        (P.z < mapMinXZ.y) || (P.z > mapMaxXZ.y);

    if (outside) dEdge = 0.0;

    vec2 p = P.xz * noiseScale + vec2(time * 0.05, -time * 0.04);
    float n1 = fbm(p);
    float n2 = fbm(p * 1.7 + 13.2);

    float beltLocal = beltWidth * mix(1.0 - noiseAmp, 1.0 + noiseAmp, n1);
    dEdge += (n2 - 0.5) * beltWidth * 0.25 * noiseAmp;

    float w = 1.0 - smoothstep(0.0, max(beltLocal, 0.001), dEdge);
    return clamp(w, 0.0, 1.0);
}

// 高度霧權重（0..1）：越靠地越濃
float heightWeight(float y)
{
    float h = max(y - fogBaseY, 0.0);
    return exp(-h / max(fogHeight, 0.001));
}

void main()
{
    vec3 baseCol  = texture(sceneColor, vUV).rgb;
    float depth01 = texture(sceneDepth, vUV).r;

    // 用 depth=1 重建遠方點，取得 ray direction（讓空氣也能 march）
    vec3 farP = reconstructWorldPos(vUV, 1.0);
    vec3 dir  = normalize(farP - cameraPos);

    // march 到哪裡結束：打到物體就到物體，沒打到就到 fogMaxDist
    float distToEnd;
    if (depth01 < 0.99999)
    {
        vec3 P = reconstructWorldPos(vUV, depth01);
        distToEnd = length(P - cameraPos);
    }
    else
    {
        distToEnd = fogMaxDist;
    }

    if (distToEnd <= 0.0001)
    {
        FragColor = vec4(baseCol, 1.0);
        return;
    }

    int steps = fogSteps;
    if (steps < 8)  steps = 8;
    if (steps > 64) steps = 64;

    float stepLen = distToEnd / float(steps);

    // jitter：減少 banding
    float j = hash21(vUV * vec2(1920.0, 1080.0) + time);
    float t = stepLen * j;

    vec3  fogAcc = vec3(0.0);
    float T = 1.0; // transmittance

    for (int i = 0; i < 64; i++)
    {
        if (i >= steps) break;
        if (T < 0.02) break;

        vec3 S = cameraPos + dir * t;

        float eW = edgeWeight(S);
        if (eW > 0.0001)
        {
            float hW = heightWeight(S.y);

            float dn = fbm(S.xz * (noiseScale * 1.6) + vec2(time * 0.02, time * 0.015));
            float noiseMul = mix(1.0 - 0.5 * noiseAmp, 1.0 + 0.8 * noiseAmp, dn);

            float sigma  = density * eW * hW * noiseMul;
            float distW = 1.0 - smoothstep(distToEnd * 0.65, distToEnd, t); // 0.65~1.0 這段淡出
            sigma *= distW;

            float localA = 1.0 - exp(-sigma * stepLen);
            localA = clamp(localA, 0.0, 1.0);

            fogAcc += T * localA * fogColor;
            T *= (1.0 - localA);
        }

        t += stepLen;
    }

    float fogOpacity = 1.0 - T;
    if (fogOpacity > fogMax && fogOpacity > 0.0001)
    {
        float s = fogMax / fogOpacity;
        fogAcc *= s;
        T = 1.0 - fogMax;
    }

    vec3 outCol = baseCol * T + fogAcc;
    FragColor = vec4(outCol, 1.0);
}
