// shaders/aurora.frag
// 固定位置 + 多條自然帶(平行) + 布簾條狀(多條細ribbon) + 飽和色
// + 自然星空 + 流星雨(事件式：隨機出現→滑過→慢慢消失) + additive，不覆蓋霧
// 效能版：
//   1) 星/流星只在 skyW 時才算
//   2) 看地面(dir.y低) 不做 raymarch
//   3) marchSteps 上限降到 36
//   4) METEOR seam 修正：不再每顆算 3 份
#version 330 core
out vec4 FragColor;
in vec2 vUV;

uniform sampler2D sceneColor; // not used (this pass additive)
uniform sampler2D sceneDepth;

uniform mat4 invViewProj;
uniform vec3 cameraPos;

uniform float auroraBaseY;
uniform float auroraTopY;
uniform float auroraDensity;

uniform int   marchSteps;
uniform float maxMarchDist;
uniform float time;

// =====================================================
// ===== PHOTO PRESET：你只改這裡就好 ====================
// =====================================================

const int NUM_BANDS = 4;

// band 分得更開（避免疊在一起）
const vec2 bandCenter[NUM_BANDS] = vec2[](
    vec2(  560.0,   60.0),
    vec2(  940.0,  170.0),
    vec2( 1340.0,  300.0),
    vec2( 1760.0,  430.0)
);

// 每條帶子方向不一樣（更自然）
const vec2 bandDir[NUM_BANDS] = vec2[](
    normalize(vec2(-0.5299, 0.8480)), // 122°
    normalize(vec2(-0.5736, 0.8192)), // 125°
    normalize(vec2(-0.6157, 0.7880)), // 128°
    normalize(vec2(-0.6691, 0.7431))  // 132°
);

// 彎曲
const float bendStrength[NUM_BANDS] = float[](
    260.0, 230.0, 205.0, 180.0
);

// 寬度
const float bandWidthMin[NUM_BANDS] = float[](50.0, 48.0, 46.0, 44.0);
const float bandWidthMax[NUM_BANDS] = float[](66.0, 64.0, 62.0, 60.0);

// gate
const float gateLo[NUM_BANDS] = float[](0.26, 0.28, 0.34, 0.38);
const float gateHi[NUM_BANDS] = float[](0.68, 0.70, 0.76, 0.80);

// 色帶偏移
const float hueShift[NUM_BANDS] = float[](0.02, 0.14, 0.26, 0.38);

// ribbon
const int   RIBBON_COUNT      = 10;
const float RIBBON_SPACING    = 50.0;
const float RIBBON_THICKNESS  = 11.5;
const float RIBBON_JITTER     = 0.62;

// 曝光：你說過曝 → 收一點
const float AURORA_EXPOSURE   = 2.05;   // 原 1.55/2.35 都偏爆
const float CORE_WHITEN       = 0.12;   // 原 0.70
const float TOP_PURPLE        = 0.58;

// =====================================================
// 極光內的「白霧」散射（托底）
// =====================================================
const vec3  AURORA_MIST_COLOR = vec3(0.95, 0.98, 1.00);
const float AURORA_MIST_GAIN  = 0.18;   // 原 0.70
const float AURORA_MIST_MAXA  = 0.016;  // 原 0.075
const float AURORA_MIST_SOFT  = 0.58;
const float AURORA_MIST_TOP   = 0.85;

// =====================================================
// 星星（固定分佈 + 微 twinkle）
// =====================================================
const float STAR_ROTATE = 0.35;

// 大顆
const float STAR_SCALE_BIG   = 250.0;
const float STAR_THRESH_BIG  = 0.9860;
const float STAR_RMIN_BIG    = 0.020;
const float STAR_RMAX_BIG    = 0.044;   // 你說想大一點點
const float STAR_BMIN_BIG    = 1.15;    // 亮一點
const float STAR_BMAX_BIG    = 2.25;
const float STAR_TW_BIG      = 1.20;    // 閃一點

// 中顆
const float STAR_SCALE_MID   = 480.0;
const float STAR_THRESH_MID  = 0.9760;
const float STAR_RMIN_MID    = 0.012;
const float STAR_RMAX_MID    = 0.030;
const float STAR_BMIN_MID    = 0.70;
const float STAR_BMAX_MID    = 1.55;
const float STAR_TW_MID      = 1.75;

// 小顆
const float STAR_SCALE_TINY  = 560.0;
const float STAR_THRESH_TINY = 0.9560;
const float STAR_RMIN_TINY   = 0.006;
const float STAR_RMAX_TINY   = 0.015;
const float STAR_BMIN_TINY   = 0.32;
const float STAR_BMAX_TINY   = 0.95;
const float STAR_TW_TINY     = 2.30;

const float STAR_GAIN = 3.40; // 原 4.15 → 避免一起爆亮

// =====================================================
// 流星雨
// =====================================================

// 方向（你要從 (+,0) 到 (0,+) 的斜上：vec2(+1,+1)；斜下：(+1,-1)）
const vec2  METEOR_DIR_UV = normalize(vec2(0.70, 0.70));

// 最大殺手：數量先砍掉（想密再 18→24）
const int   METEOR_COUNT = 60;

// 週期/生命
const float METEOR_PERIOD_MIN = 2.8;
const float METEOR_PERIOD_MAX = 7.5;
const float METEOR_LIFE_MIN   = 0.55;
const float METEOR_LIFE_MAX   = 1.35;

// 速度
const float METEOR_SPEED_MIN  = 0.18;
const float METEOR_SPEED_MAX  = 0.30;

// 尾巴
const float METEOR_LEN_MIN    = 0.028;
const float METEOR_LEN_MAX    = 0.065;
const float METEOR_W_MIN      = 0.00022;
const float METEOR_W_MAX      = 0.00060;

// 亮度（過曝就收）
const float METEOR_BRIGHT_MIN = 0.80;
const float METEOR_BRIGHT_MAX = 1.25;

// 亮頭
const float METEOR_HEAD_SIZE   = 0.00110;
const float METEOR_HEAD_BOOST2 = 0.90;

// 淡入淡出
const float METEOR_FADE_IN_FRAC  = 0.18;
const float METEOR_FADE_OUT_FRAC = 0.35;

// =====================================================
// 夜空底色（你想要那種藍）
// =====================================================
// 上方深藍、地平線偏亮一點點帶青
const vec3 SKY_TOP    = vec3(0.02, 0.06, 0.14);
const vec3 SKY_HORIZ  = vec3(0.04, 0.12, 0.28);
const vec3 SKY_EDGE_TINT = vec3(0.02, 0.26, 0.34); // 右側那種淡青光

// =====================================================
// noise
// =====================================================
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

// 效能：fbm octaves 4 -> 3
float fbm(vec2 p){
    float v = 0.0;
    float a = 0.5;
    for(int i=0;i<3;i++){
        v += a * vnoise(p);
        p *= 2.02;
        a *= 0.5;
    }
    return v;
}

// =====================================================
// reconstruct world pos
// =====================================================
vec3 reconstructWorldPos(vec2 uv, float depth01)
{
    vec2 ndcXY = uv * 2.0 - 1.0;
    float ndcZ = depth01 * 2.0 - 1.0;
    vec4 clip  = vec4(ndcXY, ndcZ, 1.0);
    vec4 world = invViewProj * clip;
    world.xyz /= world.w;
    return world.xyz;
}

float saturate(float x){ return clamp(x, 0.0, 1.0); }

// =====================================================
// Sky mapping
// =====================================================
vec2 dirToSkyUV(vec3 d)
{
    float u = atan(d.z, d.x) / (2.0 * 3.14159265) + 0.5;
    float v = acos(clamp(d.y, -1.0, 1.0)) / 3.14159265;
    return vec2(u, v);
}

// =====================================================
// Stars (Voronoi nearest)
// =====================================================
vec2 hash22(vec2 p)
{
    float n1 = hash21(p);
    float n2 = hash21(p + 19.19);
    return vec2(n1, n2);
}

void voronoiNearest(vec2 p, out float minDist, out vec2 idCell, out vec2 sitePos)
{
    vec2 g = floor(p);
    vec2 f = fract(p);

    minDist = 1e9;
    idCell  = g;
    sitePos = g;

    vec2 P = g + f;

    for(int j=-1;j<=1;j++){
        for(int i=-1;i<=1;i++){
            vec2 o    = vec2(float(i), float(j));
            vec2 cell = g + o;

            vec2 r    = hash22(cell);
            vec2 site = cell + r;

            vec2 d    = P - site;
            float dist2 = dot(d,d);

            if(dist2 < minDist){
                minDist = dist2;
                idCell  = cell;
                sitePos = site;
            }
        }
    }
    minDist = sqrt(minDist);
}

vec3 starLayer(vec2 suv, float tNow,
               float scale, float thresh,
               float rMin, float rMax,
               float bMin, float bMax,
               float twSpeed)
{
    vec2 wp = suv * scale;

    // 固定 warp（不跟時間走）
    wp += (fbm(wp * 0.05 + vec2(17.2, 9.1)) - 0.5) * 0.65;

    float d; vec2 cell; vec2 site;
    voronoiNearest(wp, d, cell, site);

    float pick = hash21(cell + vec2(7.7, 3.3));
    float hasStar = step(thresh, pick);
    if(hasStar < 0.5) return vec3(0.0);

    float r2 = hash21(cell + vec2(31.1, 9.2));
    float r3 = hash21(cell + vec2(12.8, 77.7));
    float radius = mix(rMin, rMax, r2);
    float bright = mix(bMin, bMax, r3);

    float fw = max(fwidth(d), 0.0012);

    float core = 1.0 - smoothstep(radius - fw, radius + fw, d);
    float glow = 1.0 - smoothstep(radius * 2.9 - fw, radius * 2.9 + fw, d);
    glow = pow(glow, 2.0) * 0.33;

    float phase = hash21(cell + vec2(55.5, 11.1)) * 6.28318;
    float tw = 0.72 + 0.28 * sin(tNow * twSpeed + phase);
    tw *= 0.88 + 0.12 * sin(tNow * (twSpeed * 0.63) + phase * 1.7);

    float intensity = (core + glow) * bright * tw;

    vec3 cWhite = vec3(1.00);
    vec3 cBlue  = vec3(0.70, 0.82, 1.00);
    vec3 cWarm  = vec3(1.00, 0.88, 0.72);
    vec3 c = mix(cWarm, cBlue, hash21(cell + vec2(2.1, 9.9)));
    c = mix(cWhite, c, 0.35);

    return c * intensity;
}

vec3 starsColor(vec3 rayDir, float tNow)
{
    float skyMask = smoothstep(0.08, 0.34, rayDir.y);
    if (skyMask <= 0.0001) return vec3(0.0);

    float ca = cos(STAR_ROTATE), sa = sin(STAR_ROTATE);
    rayDir.xz = mat2(ca, -sa, sa, ca) * rayDir.xz;

    vec2 suv = dirToSkyUV(rayDir);

    vec3 big  = starLayer(suv + vec2(0.13, 0.07), tNow,
                          STAR_SCALE_BIG, STAR_THRESH_BIG,
                          STAR_RMIN_BIG, STAR_RMAX_BIG,
                          STAR_BMIN_BIG, STAR_BMAX_BIG,
                          STAR_TW_BIG);

    vec3 mid  = starLayer(suv + vec2(0.41, 0.19), tNow,
                          STAR_SCALE_MID, STAR_THRESH_MID,
                          STAR_RMIN_MID, STAR_RMAX_MID,
                          STAR_BMIN_MID, STAR_BMAX_MID,
                          STAR_TW_MID);

    vec3 tiny = starLayer(suv + vec2(0.73, 0.33), tNow,
                          STAR_SCALE_TINY, STAR_THRESH_TINY,
                          STAR_RMIN_TINY, STAR_RMAX_TINY,
                          STAR_BMIN_TINY, STAR_BMAX_TINY,
                          STAR_TW_TINY);

    vec3 s = (big + mid + tiny) * STAR_GAIN * skyMask;
    s = pow(s, vec3(0.88)); // 稍微收一點避免爆亮
    return s;
}

// =====================================================
// Meteors (event-based)
// =====================================================
float hash11(float n){ return fract(sin(n) * 43758.5453123); }

float distToSeg(vec2 p, vec2 a, vec2 b, out float t01)
{
    vec2 ab = b - a;
    float l2 = max(dot(ab,ab), 1e-6);
    float t = dot(p - a, ab) / l2;
    t01 = clamp(t, 0.0, 1.0);
    vec2 c = a + ab * t01;
    return length(p - c);
}

vec3 oneMeteor(vec2 suv, float tNow, float seedBase)
{
    float r0 = hash11(seedBase + 1.0);
    float r1 = hash11(seedBase + 2.0);
    float r2 = hash11(seedBase + 3.0);
    float r3 = hash11(seedBase + 4.0);
    float r4 = hash11(seedBase + 5.0);
    float r5 = hash11(seedBase + 6.0);
    float rc = hash11(seedBase + 13.37);

    float period = mix(METEOR_PERIOD_MIN, METEOR_PERIOD_MAX, r0);
    float life   = mix(METEOR_LIFE_MIN,   METEOR_LIFE_MAX,   r1);
    life = min(life, period * 0.75);

    float phase0 = r2 * period;
    float lt = mod(tNow + phase0, period);
    if (lt > life) return vec3(0.0);

    float fadeIn  = max(life * METEOR_FADE_IN_FRAC,  0.04);
    float fadeOut = max(life * METEOR_FADE_OUT_FRAC, 0.06);

    float aIn  = smoothstep(0.0,         fadeIn,  lt);
    float aOut = 1.0 - smoothstep(life - fadeOut, life, lt);
    float alive = aIn * aOut;

    // 起點：天空區
    vec2 start = vec2(r3, mix(0.08, 0.55, r4));

    float speed  = mix(METEOR_SPEED_MIN, METEOR_SPEED_MAX, r5);
    float len    = mix(METEOR_LEN_MIN,   METEOR_LEN_MAX,   hash11(seedBase + 7.7));
    float w      = mix(METEOR_W_MIN,     METEOR_W_MAX,     hash11(seedBase + 8.8));
    float bright = mix(METEOR_BRIGHT_MIN,METEOR_BRIGHT_MAX,hash11(seedBase + 9.9));

    vec2 head = start + METEOR_DIR_UV * (lt * speed);
    vec2 tail = head - METEOR_DIR_UV * len;

    // seam 修正（不再算三份）：把 suv.x 拉到離流星中心最近的那個週期
    vec2 p = suv;
    float cx = 0.5 * (head.x + tail.x);
    if (p.x - cx >  0.5) p.x -= 1.0;
    if (p.x - cx < -0.5) p.x += 1.0;

    float t01;
    float d = distToSeg(p, tail, head, t01);

    float fw = max(fwidth(d), 1e-4);
    float ww = max(w, fw * 1.5);

    float line = 1.0 - smoothstep(0.0, ww, d);

    float along = pow(t01, 2.4);
    float tailFade = exp(-3.2 * (1.0 - t01));

    float headD = length(p - head);
    float headGlow = exp(-pow(headD / METEOR_HEAD_SIZE, 2.0)) * METEOR_HEAD_BOOST2;

    vec3 cCold  = vec3(0.80, 0.88, 1.00);
    vec3 cBlue  = vec3(0.55, 0.75, 1.10);
    vec3 cGreen = vec3(0.65, 1.05, 0.85);
    vec3 cPurple= vec3(0.95, 0.70, 1.20);
    vec3 cWarm  = vec3(1.10, 0.92, 0.75);

    vec3 c = cCold;
    c = mix(c, cBlue,   smoothstep(0.10, 0.30, rc) - smoothstep(0.30, 0.50, rc));
    c = mix(c, cGreen,  smoothstep(0.30, 0.50, rc) - smoothstep(0.50, 0.70, rc));
    c = mix(c, cPurple, smoothstep(0.50, 0.70, rc) - smoothstep(0.70, 0.88, rc));
    c = mix(c, cWarm,   smoothstep(0.78, 0.98, rc));
    c = mix(c, vec3(1.0), 0.25);

    float intensity = (line * (0.20 + 1.20 * along) * tailFade + headGlow) * alive * bright;
    return c * intensity;
}

vec3 meteorsColor(vec3 rayDir, float tNow)
{
    float skyMask = smoothstep(0.08, 0.34, rayDir.y);
    if (skyMask <= 0.0001) return vec3(0.0);

    float ca = cos(STAR_ROTATE), sa = sin(STAR_ROTATE);
    rayDir.xz = mat2(ca, -sa, sa, ca) * rayDir.xz;

    vec2 suv = dirToSkyUV(rayDir);

    vec3 acc = vec3(0.0);
    for (int i = 0; i < METEOR_COUNT; i++)
    {
        float seed = float(i) * 37.17 + 19.23;
        acc += oneMeteor(suv, tNow, seed);
    }

    // 防爆亮（additive pass）
    acc = min(acc, vec3(4.0));
    return acc * skyMask;
}

// =====================================================
// Ribbon bundle
// =====================================================
float ribbonBundle(float w, float s, float centerW, float bandWidth, float seed)
{
    float base = 1.0 - abs(w - centerW) / max(bandWidth, 1e-3);
    base = saturate(base);
    base = pow(base, 1.2);

    float spacing = RIBBON_SPACING * mix(0.85, 1.15, fbm(vec2(s * 0.0016 + seed, time * 0.02)));
    float thick   = RIBBON_THICKNESS * mix(0.80, 1.25, fbm(vec2(s * 0.0023 + seed * 2.0, time * 0.03)));

    float drift = (fbm(vec2(s * 0.0022 + seed, time * 0.06)) - 0.5) * spacing * RIBBON_JITTER;

    float w2 = w - centerW - drift;
    float idx = w2 / max(spacing, 1e-3);

    float stripe = 0.0;
    for(int k=-2;k<=2;k++)
    {
        float kk = floor(idx) + float(k);
        float rSeed = hash21(vec2(kk, seed));
        float amp   = mix(0.55, 1.10, rSeed);

        float dw = abs(w2 - kk * spacing);
        float g  = exp(-(dw*dw) / (2.0 * thick * thick));
        stripe = max(stripe, g * amp);
    }

    return base * stripe;
}

// =====================================================
// Band density
// =====================================================
float bandDensity(vec3 P, vec3 rayDir,
                  vec2 CENTER_XZ, vec2 DIR_IN,
                  float BEND, float WMIN, float WMAX,
                  float GLO, float GHI,
                  float hueShiftLocal,
                  out float hueVar01)
{
    float h01 = (P.y - auroraBaseY) / (auroraTopY - auroraBaseY);
    h01 = clamp(h01, 0.0, 1.0);

    float hMask = smoothstep(0.04, 0.20, h01) * (1.0 - smoothstep(0.78, 1.00, h01));
    if (hMask <= 0.0005) { hueVar01 = 0.0; return 0.0; }

    float viewMask = 1.0 - smoothstep(0.28, 0.78, rayDir.y);
    viewMask = clamp(viewMask, 0.0, 1.0);
    if (viewMask <= 0.0005) { hueVar01 = 0.0; return 0.0; }

    vec2 dir  = normalize(DIR_IN);
    vec2 perp = vec2(-dir.y, dir.x);

    vec2 xz = P.xz - CENTER_XZ;

    float warp1 = fbm(xz * 0.0026 + vec2( time * 0.018, -time * 0.014));
    float warp2 = fbm(xz * 0.0048 + vec2(-time * 0.026,  time * 0.010));
    xz += perp * (warp1 - 0.5) * BEND;
    xz += dir  * (warp2 - 0.5) * (BEND * 0.25);

    float s = dot(xz, dir);
    float w = dot(xz, perp);

    float seed  = hash21(CENTER_XZ * 0.01);
    float phase = seed * 23.0;

    float centerW = (fbm(vec2(s * 0.0022, time * 0.06 + phase)) - 0.5) * (BEND * 1.05);
    float bandW   = mix(WMIN, WMAX, fbm(vec2(s * 0.0013 + 10.0, time * 0.02 + phase * 0.35)));

    float gate = fbm(vec2(s * 0.00095 + time * 0.018 + phase * 0.20, 3.7));
    gate = smoothstep(GLO, GHI, gate);

    float stripes = ribbonBundle(w, s, centerW, bandW, seed);
    if (stripes <= 0.0005) { hueVar01 = 0.0; return 0.0; }

    float curtain = mix(0.72, 1.18, smoothstep(0.20, 0.95, h01));

    float flow = time * 0.28;
    float glow = fbm(vec2(s * 0.0016 + flow, time * 0.04 + phase * 0.10));
    glow = mix(0.60, 1.05, glow);

    float shimmer = fbm(vec2(s * 0.010 + flow * 2.0 + phase * 0.15, w * 0.002));
    shimmer = mix(0.88, 1.06, shimmer);

    float d = stripes * glow * shimmer * curtain * gate * hMask * viewMask;

    float hueN = fbm((P.xz - CENTER_XZ) * 0.0028 + vec2(time * 0.018 + phase * 0.08, -time * 0.014));
    hueVar01 = clamp(0.40 * h01 + 0.60 * hueN + hueShiftLocal, 0.0, 1.0);

    return d;
}

// =====================================================
// Sky base color (你的藍)
// =====================================================
vec3 skyBaseColor(vec3 dir, float skyW)
{
    // dir.y: -1..1 → 0..1
    float y01 = saturate(dir.y * 0.5 + 0.5);

    // 上方深藍、地平線略亮
    float t = pow(y01, 1.35);
    vec3 sky = mix(SKY_HORIZ, SKY_TOP, t);

    // 右側淡青的 edge 光（你那張圖右側偏青）
    float edge = pow(saturate((vUV.x - 0.86) / 0.14), 1.8);
    sky += SKY_EDGE_TINT * edge * 0.40;

    // 很淡的 haze（只作用 sky）
    sky += vec3(0.02) * pow(saturate(dir.y), 1.6);

    return sky * skyW;
}

void main()
{
    float depth01 = texture(sceneDepth, vUV).r;

    vec3 farP = reconstructWorldPos(vUV, 1.0);
    vec3 dir  = normalize(farP - cameraPos);

    // skyW：只有背景(沒幾何)才是 1
    float skyW = smoothstep(0.985, 1.0, depth01);

    // ✅ 看向地面/水平(dir.y低) 不做 raymarch（省爆）
    bool doMarch = (dir.y > 0.02);

    // ✅ 星/流星：只在 sky 才算（地面像素直接 0）
    vec3 stars = vec3(0.0);
    if (skyW > 0.0001) {
        stars = (starsColor(dir, time) + meteorsColor(dir, time)) * skyW;
    }

    // ✅ 夜空底色（你想要那種藍）
    vec3 skyBase = skyBaseColor(dir, skyW);

    float distEnd;
    if (depth01 < 0.99999)
    {
        vec3 hitP = reconstructWorldPos(vUV, depth01);
        distEnd = length(hitP - cameraPos);
    }
    else
    {
        distEnd = maxMarchDist;
    }

    // 如果完全是天空且不 march，就只輸出 sky + stars
    if (!doMarch || distEnd <= 0.001)
    {
        FragColor = vec4(skyBase + stars, 1.0);
        return;
    }

    // =====================================================
    // ✅ Raymarch Aurora
    // =====================================================
    vec3  accCol = vec3(0.0);
    float T = 1.0;

    int steps = clamp(marchSteps, 16, 36);   // ✅ 上限 64 -> 36
    float stepLen = distEnd / float(steps);

    float j = hash21(vUV * vec2(1920.0, 1080.0) + time);
    float tRay = stepLen * j;

    for (int i = 0; i < 64; i++)
    {
        if (i >= steps) break;
        if (T < 0.02) break;

        vec3 S = cameraPos + dir * tRay;

        if (S.y >= auroraBaseY && S.y <= auroraTopY)
        {
            float dSum   = 0.0;
            float hueMix = 0.0;
            float wSum   = 0.0;

            for(int b=0;b<NUM_BANDS;b++){
                float hueB;
                float dB = bandDensity(S, dir,
                                       bandCenter[b], bandDir[b],
                                       bendStrength[b],
                                       bandWidthMin[b], bandWidthMax[b],
                                       gateLo[b], gateHi[b],
                                       hueShift[b],
                                       hueB);
                if(dB > 0.0){
                    float wb = dB;
                    hueMix += hueB * wb;
                    wSum   += wb;
                    dSum   += dB;
                }
            }

            if (dSum > 0.001)
            {
                float hue01 = (wSum > 0.0) ? (hueMix / wSum) : 0.0;
                float inten = clamp(dSum, 0.0, 1.0);

                float sigma = dSum * auroraDensity;
                float a = 1.0 - exp(-sigma * stepLen);
                a = clamp(a, 0.0, 1.0);

                // ✅ 過曝主因：alpha 太大 → 收
                a = min(a, 0.22);

                vec3 green  = vec3(0.04, 1.85, 0.18);
                vec3 yellow = vec3(1.70, 1.35, 0.20);
                vec3 pink   = vec3(1.35, 0.10, 0.95);
                vec3 purple = vec3(0.42, 0.06, 1.70);


                vec3 col = green;
                col = mix(col, yellow, smoothstep(0.15, 0.45, hue01));
                col = mix(col, pink,   smoothstep(0.45, 0.78, hue01));
                col = mix(col, purple, smoothstep(0.70, 1.00, hue01));

                float core = smoothstep(0.30, 0.95, inten);
                col = mix(col, vec3(1.45, 1.35, 0.78), core * CORE_WHITEN);

                float h01 = clamp((S.y - auroraBaseY) / (auroraTopY - auroraBaseY), 0.0, 1.0);
                float topTint = smoothstep(0.35, 1.00, h01) * smoothstep(0.08, 0.65, inten);
                col = mix(col, pink,   topTint * 0.40);
                col = mix(col, purple, topTint * TOP_PURPLE);

                col *= mix(1.20, 2.60, core);

                // 高度越高越淡一些（避免整片爆白）
                float lumMul = mix(1.25, 0.60, smoothstep(0.0, 1.0, h01));
                col *= lumMul;

                // ✅ 最終 alpha 再收
                a = min(a, 0.18);

                accCol += T * a * col;
                T *= (1.0 - a);

                // mist
                float mistH = pow(saturate(h01), AURORA_MIST_TOP);
                float mistI = pow(saturate(inten), AURORA_MIST_SOFT);
                float mistSigma = dSum * auroraDensity * AURORA_MIST_GAIN * (0.65 + 0.55 * mistH);

                float aMist = 1.0 - exp(-mistSigma * stepLen);
                aMist = clamp(aMist * (0.55 + 0.65 * mistI), 0.0, AURORA_MIST_MAXA);

                accCol += T * aMist * AURORA_MIST_COLOR;
                T *= (1.0 - aMist);
            }
        }

        tRay += stepLen;
    }

    float alpha = clamp(1.0 - T, 0.0, 1.0);
    alpha = pow(alpha, 1.10);

    // ✅ 曝光 + tonemap（收過曝）
    accCol *= (1.10 * AURORA_EXPOSURE);
    accCol = accCol / (1.0 + accCol);

    // 星星在極光強的地方稍微被吃掉
    stars *= (1.0 - alpha * 0.22);

    // ✅ 合成：skyBase + aurora + stars
    vec3 outRGB = skyBase + accCol * alpha + stars;

    FragColor = vec4(outRGB, 1.0);
}
