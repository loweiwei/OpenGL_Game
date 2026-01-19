#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "WorldManager.hpp"
#include "Shader.h"
#include "Camera.h"
#include "Player.hpp"
#include "Terrain.hpp"
#include "Collision.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// =====================================================
// Global Objects & State
// =====================================================
Camera camera(glm::vec3(0.0f, 25.0f, 6.0f));
Player player(glm::vec3(40.0f, 20.0f, 40.0f));

float lastX        = 1000.0f / 2.0f;
float lastY        = 800.0f  / 2.0f;
float deltaTime    = 0.0f;
float lastFrame    = 0.0f;
bool  firstMouse   = true;
bool  menuOpen     = false;   // 右鍵「說明菜單」是否開啟

// =====================================================
// Utility: Menu Help
// =====================================================
static void printMenuHelp()
{
    std::cout << "\n=== RIGHT-CLICK MENU (Console) ===\n"
              << "[Lighting]   \n"
              << "Robot Actions: 1=Stop 2=Walk 3=Run 4=ClaspedHands\n"
              << "Joint Select : .=Next  ,=Prev\n"
              << "Rotate Joint : x/X y/Y z/Z  (±5 deg)\n"
              << "Satellite    : r=toggle revolution, t=toggle rotation\n"
              << "Axis         : u=X  v=Y  w=Z\n"
              << "Right Click  : toggle menu (open/close)\n"
              << "=================================\n\n";
}

// =====================================================
// Utility: Texture Loader（2D 貼圖載入）
// =====================================================
unsigned int LoadTexture2D(const char* path)
{
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // 基本參數
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int w, h, nChannels;
    unsigned char* data = stbi_load(path, &w, &h, &nChannels, 0);
    if (data)
    {
        GLenum format = (nChannels == 4 ? GL_RGBA : GL_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << "\n";
    }

    stbi_image_free(data);
    return texID;
}

// =====================================================
// Callbacks
// =====================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // 菜單打開時，只更新 lastX/lastY，不做鏡頭轉動
    if (menuOpen) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        return;
    }

    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float dx = (float)xpos - lastX;
    float dy = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    int L = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int R = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    // 左鍵拖曳：旋轉相機
    if (L == GLFW_PRESS && R == GLFW_RELEASE) {
        camera.ProcessMouseMovement(dx, dy);
    }
    // 左+右鍵：縮放（距離）
    else if (L == GLFW_PRESS && R == GLFW_PRESS) {
        camera.ProcessMouseScroll(dy * 0.02f);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        menuOpen = !menuOpen;

        if (menuOpen) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            printMenuHelp();
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // 開菜單時不吃滾輪
    if (!menuOpen)
        camera.ProcessMouseScroll((float)yoffset);
}

// 鍵盤輸入：關節控制、動作切換、衛星控制
void char_callback(GLFWwindow* window, unsigned int codepoint)
{
    switch ((char)codepoint)
    {
    case '.': player.robot.selectNext(); break;
    case ',': player.robot.selectPrev(); break;

    case 'x': player.robot.addAngle('x', +5); break;
    case 'X': player.robot.addAngle('x', -5); break;
    case 'y': player.robot.addAngle('y', +5); break;
    case 'Y': player.robot.addAngle('y', -5); break;
    case 'z': player.robot.addAngle('z', +5); break;
    case 'Z': player.robot.addAngle('z', -5); break;

    // Robot actions（Figure2：Stop/Walk/Run/Clasped-Hands/Bow）
    case '1': player.robot.action = RobotAction::Stop; break;
    case '2': player.robot.action = RobotAction::Walk; break;
    case '3': player.robot.action = RobotAction::Run; break;
    case '4': player.robot.action = RobotAction::Bow;  break;

    // Satellite control（Figure2：Revolution/Rotation OnOff + Axis）
    case 'r': player.robot.sat.revolutionOn = !player.robot.sat.revolutionOn; break;
    case 't': player.robot.sat.rotationOn   = !player.robot.sat.rotationOn;   break;
    case 'u': player.robot.sat.axis = glm::vec3(1,0,0); break; // X
    case 'v': player.robot.sat.axis = glm::vec3(0,1,0); break; // Y
    case 'w': player.robot.sat.axis = glm::vec3(0,0,1); break; // Z
    }
}

// =====================================================
// Input: WASD + Space + 自動動作狀態
// =====================================================
void processInput(GLFWwindow* window)
{
    // 以相機朝向為基準，計算平面上的前 / 右方向
    glm::vec3 camForward = camera.Front;
    camForward.y = 0;
    if (glm::length(camForward) > 0.0001f)
        camForward = glm::normalize(camForward);

    glm::vec3 camRight = glm::normalize(glm::cross(camForward, glm::vec3(0, 1, 0)));
    glm::vec3 move(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += camForward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= camForward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= camRight;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += camRight;

    // Space：跳躍（只在地面時觸發）
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (!player.isInAir)
        {
            player.isInAir        = true;
            player.verticalVelocity = player.jumpSpeed;
        }
    }

    // 角色位移
    if (glm::length(move) > 0.0001f)
    {
        move = glm::normalize(move);
        player.Velocity += move * player.Speed;

        // TPS：角色永遠面向「相機的背面」方向
        player.Yaw = glm::degrees(std::atan2(camForward.x, camForward.z));
    }

    // 自動切換 Walk / Stop，不干擾其它原地動作（例如 Bow）
    const bool isMovingInput = (glm::length(move) > 0.0001f);

    if (isMovingInput) {
        player.robot.action = RobotAction::Walk;   // 只要有移動輸入 → Walk
    } else {
        if (player.robot.action == RobotAction::Walk)
            player.robot.action = RobotAction::Stop; // 停下來：Walk → Stop
    }
}

// =====================================================
// Main
// =====================================================
int main()
{
    // -------------------------------------------------
    // 1. Initialize GLFW & Window
    // -------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1000, 800, "Snow Field", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 註冊 Callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCharCallback(window, char_callback);

    // 鎖定滑鼠（TPS 視角）
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // -------------------------------------------------
    // 2. Shaders
    // -------------------------------------------------
    Shader terrainShader      ("shaders/terrain.vert",       "shaders/terrain.frag");
    Shader cubeShader         ("shaders/cube.vert",          "shaders/cube.frag");
    Shader waterShader        ("shaders/water.vert",         "shaders/water.frag");
    Shader icemountainShader  ("shaders/IceMountain.vert",   "shaders/IceMountain.frag");
    Shader flagShader         ("shaders/flag.vert",          "shaders/flag.frag");
    Shader waterSurfaceShader ("shaders/water_surface.vert", "shaders/water_surface.frag");
    Shader fogPostShader      ("shaders/fog_post.vert",      "shaders/fog_post.frag");
    Shader humanShader        ("shaders/human.vert",         "shaders/human.frag");
    Shader smallObjectShader  ("shaders/small_object.vert",  "shaders/small_object.frag");
    Shader auroraShader       ("shaders/aurora.vert",        "shaders/aurora.frag");

    // 載入機器人模型
    player.InitModel();

    // -------------------------------------------------
    // 3. Load Textures
    // -------------------------------------------------
    unsigned int terrainTexID = LoadTexture2D("assets/textures/snow.jpg");
    unsigned int iceTexID     = LoadTexture2D("assets/textures/ice_rock.jpg");
    unsigned int snowTexID    = LoadTexture2D("assets/textures/ice_snow.jpg");

    // -------------------------------------------------
    // 4. World Generation
    // -------------------------------------------------
    WorldManager world(600.0f);

    world.addTile(0, 0, START_AREA);
    world.addTile(0, 1, BIG_ICE_MOUNTAIN_FLOATING);
    world.addTile(0, 2, MOUNTAIN);

    world.generate();
    world.setGlobalTextures(terrainTexID, iceTexID, snowTexID);

    // =================================================
    // 5. Main Loop
    // =================================================
    while (!glfwWindowShouldClose(window))
    {
        // ---- Time ----
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // -------------------------------------------------
        // Update：Input → Physics/Collision → Camera
        // -------------------------------------------------
        processInput(window);

        // 更新所有 OBB（場景物件）
        for (auto obj : world.getObjects())
            obj->updateWorldOBB();

        player.isOnPlatform = false;

        // 玩家移動（含平台落地 + OBB 碰撞）
        player.applyMovement(deltaTime, world.getObjects());

        // 更新玩家 OBB
        player.updateOBB();

        // 若不在平台上 → 根據地形高度強制貼地
        if (!player.isOnPlatform)
        {
            float groundY = world.getHeight(player.Position.x,
                                            player.Position.z);

            if (player.Position.y < groundY)
            {
                player.Position.y     = groundY;
                player.verticalVelocity = 0;
                player.isInAir        = false;
            }
        }

        // 相機 TPS 跟隨玩家
        camera.FollowPlayer(player.Position);

        // 相機與地面碰撞：避免鑽進雪地裡
        float cy = world.getHeight(camera.Position.x, camera.Position.z);
        if (camera.Position.y < cy + 1.0f)
            camera.Position.y = cy + 1.0f;

        // -------------------------------------------------
        // Rendering
        // -------------------------------------------------
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),
            1000.0f / 800.0f,
            0.1f, 600.0f
        );

        // ---- Global Shader Uniforms ----
        // Terrain
        terrainShader.use();
        terrainShader.setInt("texture1", 0);

        // Ice Mountain
        icemountainShader.use();
        icemountainShader.setInt("iceTex",  1);
        icemountainShader.setInt("snowTex", 2);

        // Flag
        flagShader.use();
        flagShader.setMat4("view",       view);
        flagShader.setMat4("projection", projection);

        // Water Surface
        waterSurfaceShader.use();

        // Small Objects / Human
        smallObjectShader.use();
        smallObjectShader.setMat4("view",       view);
        smallObjectShader.setMat4("projection", projection);
        smallObjectShader.setVec3("lightPos", glm::vec3(200, 500, 200));
        smallObjectShader.setVec3("viewPos",  camera.Position);

        // ---- World Rendering ----
        world.render(
            terrainShader, cubeShader, waterShader,
            icemountainShader, flagShader,
            waterSurfaceShader, fogPostShader,
            smallObjectShader, humanShader,
            auroraShader,
            player,
            camera, (float)glfwGetTime(),
            camera.Position, view, projection
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
