#pragma once
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <cmath>

class PerlinNoise {
public:

    // ★ 有 seed 的 constructor → 地圖固定
    PerlinNoise(unsigned int seed) {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0); // 0..255

        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);

        p.insert(p.end(), p.begin(), p.end());
    }

    // ★ 無參數＝隨機地圖，也可以留著
    PerlinNoise()
        : PerlinNoise(std::random_device{}()) {}

    // ★ 2D noise 函式
    double noise(double x, double y) const {
        int xi = (int)floor(x) & 255;
        int yi = (int)floor(y) & 255;

        double xf = x - floor(x);
        double yf = y - floor(y);

        double u = fade(xf);
        double v = fade(yf);

        int aa = p[p[xi] + yi];
        int ab = p[p[xi] + yi + 1];
        int ba = p[p[xi + 1] + yi];
        int bb = p[p[xi + 1] + yi + 1];

        double x1 = lerp(grad(aa, xf, yf),     grad(ba, xf - 1, yf),     u);
        double x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);

        return (lerp(x1, x2, v) + 1) / 2.0;   // normalize to 0~1
    }

private:
    std::vector<int> p;

    static double fade(double t) { return t*t*t*(t*(t*6 - 15) + 10); }
    static double lerp(double a, double b, double t) { return a + t * (b - a); }
    
    static double grad(int hash, double x, double y) {
        switch (hash & 3) {
            case 0: return  x + y;
            case 1: return -x + y;
            case 2: return  x - y;
            case 3: return -x - y;
        }
        return 0;
    }
};
