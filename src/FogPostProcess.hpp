#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader;

class FogPostProcess {
public:
    FogPostProcess();
    ~FogPostProcess();

    // 初始化（視窗大小）
    bool init(int width, int height);

    // 視窗 resize 時重建貼圖
    void resize(int width, int height);

    // Pass A：開始把場景畫進 FBO（你在這之後照常畫 terrain / objects）
    void beginScene();

    // Pass B：把 FBO 結果 + depth 做 fog 後處理，輸出到螢幕
    // 注意：你要傳 view/projection/cameraPos
    void renderToScreen(
        Shader& fogPostShader,
        Shader& auroraShader,
        const glm::mat4& view,
        const glm::mat4& projection,
        const glm::vec3& cameraPos,
        float timeSeconds
    );

    // 參數（你可以在外面改）
    void setMapBounds(const glm::vec2& minXZ, const glm::vec2& maxXZ);
    //void setFogParams(float beltWidth, const glm::vec3& fogColor, float fogMax);
    //void setNoiseParams(float noiseScale, float noiseAmp);
    //void setVolumeParams(float density, float fogBaseY, float fogHeight, int steps);
    GLuint getColorTex() const { return m_colorTex; }
    GLuint getDepthTex() const { return m_depthTex; }

private:
    void destroy();
    void buildQuad();

private:
    int m_w = 0, m_h = 0;
    GLuint m_fbo = 0;
    GLuint m_colorTex = 0;
    GLuint m_depthTex = 0;
    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;
    // uniforms defaults
    glm::vec2 m_mapMinXZ = glm::vec2(-40.0f, -40.0f);
    glm::vec2 m_mapMaxXZ = glm::vec2(640.0f, 1840.0f); // 你的地圖

    //float m_beltWidth = 260.0f;
    //glm::vec3 m_fogColor = glm::vec3(0.78f, 0.83f, 0.90f);
    //float m_fogMax = 0.85f;
    //float m_noiseScale = 0.03f;
    //float m_noiseAmp   = 0.75f;
    //float m_density  = 0.018f;
    //float m_fogBaseY = 0.0f;
    //float m_fogHeight= 100.0f;
    //int   m_steps    = 28;
    
};
