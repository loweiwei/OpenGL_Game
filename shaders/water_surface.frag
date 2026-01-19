#version 330 core

in vec2 vUV;
in float vHeight;

uniform float waterLevel;
uniform float time;

out vec4 FragColor;

void main()
{
    // ----------------------------
    // 1) 顏色：更像水、少金屬感
    // ----------------------------
    vec3 shallow = vec3(0.30, 0.62, 0.88);
    vec3 deep    = vec3(0.02, 0.10, 0.22);

    // ----------------------------
    // 2) 柔和波動：低頻、連續
    // ----------------------------
    vec2 uv = vUV;

    // 低頻的流動感（不要太高頻）
    float w1 = sin(uv.x * 6.0  + time * 0.9);
    float w2 = sin(uv.y * 7.0  - time * 0.7);
    float w3 = sin((uv.x + uv.y) * 5.0 + time * 0.6);

    float wave = (w1 + w2 + w3) * (1.0 / 3.0);  // -1~1
    wave *= 0.02;                                // 振幅（小就不碎）

    // 少量 UV 位移（讓水在「流」）
    uv += vec2(wave, -wave * 0.4);

    // ----------------------------
    // 3) 深度：水面 - 地形高度（加一點點波動）
    // ----------------------------
    float depth = waterLevel - vHeight;
    depth += wave * 0.6;                 // 很弱即可
    depth = clamp(depth / 5.0, 0.0, 1.0);

    vec3 color = mix(shallow, deep, depth);

    // 整體再深一點點
    color *= 0.90;

    // ----------------------------
    // 4) 柔和水面紋理（不金屬、不碎）
    //   用「超柔的亮暗」取代尖銳高光
    // ----------------------------
    float rip = sin(uv.x * 10.0 + time * 1.2) * sin(uv.y * 9.0 - time * 1.0);
    rip = 0.5 + 0.5 * rip;               // 0..1

    // 抗鋸齒：把過細的變化壓掉，減少碎碎感
    float aa = clamp(fwidth(uv.x * 10.0) + fwidth(uv.y * 9.0), 0.0, 1.0);
    rip = mix(rip, 0.5, aa * 0.8);

    // 非金屬的柔亮：只在淺水比較明顯
    float sheen = (1.0 - depth) * 0.08;  // 很淡
    color += vec3((rip - 0.5) * sheen);

    FragColor = vec4(color, 1.0);
}
