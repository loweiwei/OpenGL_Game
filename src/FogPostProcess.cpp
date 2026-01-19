#include "FogPostProcess.hpp"
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// 你自己的 Shader class（需提供 use(), setInt(), setMat4(), setVec2(), setVec3(), setFloat()）
#include "Shader.h"

FogPostProcess::FogPostProcess() {}
FogPostProcess::~FogPostProcess() { destroy(); }

void FogPostProcess::destroy()
{
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    m_quadVBO = m_quadVAO = 0;

    if (m_depthTex) glDeleteTextures(1, &m_depthTex);
    if (m_colorTex) glDeleteTextures(1, &m_colorTex);
    m_depthTex = m_colorTex = 0;

    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    m_fbo = 0;
}

bool FogPostProcess::init(int width, int height)
{
    m_w = width;
    m_h = height;

    buildQuad();

    // FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Color texture
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    // Depth texture (可取樣)
    glGenTextures(1, &m_depthTex);
    glBindTexture(GL_TEXTURE_2D, m_depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_w, m_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // 重要：不要做深度比較模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "[FogPostProcess] FBO incomplete!\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void FogPostProcess::resize(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    if (width == m_w && height == m_h) return;

    m_w = width;
    m_h = height;

    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, m_depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_w, m_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void FogPostProcess::buildQuad()
{
    if (m_quadVAO != 0) return;

    // 2 triangles, clip-space positions + UV
    // aPos (x,y), aUV (u,v)
    float quadVerts[] = {
        //  x     y      u    v
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,

        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // aPos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // aUV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void FogPostProcess::beginScene()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_w, m_h);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glClearColor(0.04f, 0.15f, 0.46f, 1.2f); // 你喜歡的天空底色
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// FogPostProcess.cpp 內：renderToScreen() 完整版本
void FogPostProcess::renderToScreen(
    Shader& fogPostShader,
    Shader& auroraShader,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& cameraPos,
    float timeSeconds
)
{
    // 回到螢幕
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_w, m_h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    // bind scene textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_depthTex);

    glm::mat4 invViewProj = glm::inverse(projection * view);

    glBindVertexArray(m_quadVAO);

    // =========================================================
    // PASS 1：FOG（先直接輸出到螢幕，不要 blend）
    // =========================================================
    glDisable(GL_BLEND);

    fogPostShader.use();
    fogPostShader.setInt("sceneColor", 0);
    fogPostShader.setInt("sceneDepth", 1);
    fogPostShader.setMat4("invViewProj", invViewProj);
    fogPostShader.setVec3("cameraPos", cameraPos);

    fogPostShader.setVec2("mapMinXZ", m_mapMinXZ);
    fogPostShader.setVec2("mapMaxXZ", m_mapMaxXZ);
    fogPostShader.setFloat("beltWidth", 40.0f);
    fogPostShader.setFloat("density", 0.062f);
    fogPostShader.setFloat("fogBaseY", 0.0f);
    fogPostShader.setFloat("fogHeight", 40.0f);
    fogPostShader.setInt("fogSteps", 32);
    fogPostShader.setFloat("noiseScale", 0.018f);
    fogPostShader.setFloat("noiseAmp", 0.65f);
    fogPostShader.setFloat("fogMax", 0.42f);
    fogPostShader.setVec3("fogColor", glm::vec3(0.86f, 0.92f, 1.00f));
    fogPostShader.setFloat("time", timeSeconds);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // =========================================================
    // PASS 2：AURORA（疊加在「已經有霧的畫面」上）
    // =========================================================
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // ✅ alpha 控制加亮量（很適合極光）

    auroraShader.use();
    auroraShader.setInt("sceneColor", 0);
    auroraShader.setInt("sceneDepth", 1);
    auroraShader.setMat4("invViewProj", invViewProj);
    auroraShader.setVec3("cameraPos", cameraPos);

    auroraShader.setFloat("auroraBaseY", 100.0f);
    auroraShader.setFloat("auroraTopY", 300.0f);
    auroraShader.setFloat("auroraDensity", 0.40f);
    auroraShader.setInt("marchSteps",40);
    auroraShader.setFloat("maxMarchDist", 1200.0f);
    auroraShader.setFloat("time", timeSeconds);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // cleanup
    glDisable(GL_BLEND);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 還原（保險）
    glDepthMask(GL_TRUE);
}

void FogPostProcess::setMapBounds(const glm::vec2& minXZ, const glm::vec2& maxXZ)
{
    m_mapMinXZ = minXZ;
    m_mapMaxXZ = maxXZ;
}


