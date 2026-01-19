//#define GLM_ENABLE_EXPERIMENTAL

#include "WorldManager.hpp"

#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ==================================================
// Constructor / Destructor
// ==================================================
WorldManager::WorldManager(float tileSize)
    : tileSize(tileSize)
{
}

WorldManager::~WorldManager()
{
    for (auto& tile : tiles)
    {
        delete tile.terrain;
        delete tile.water;
        delete tile.waterManager;

        for (auto& ice : tile.iceCubes)
            delete ice.mesh;

        for (auto* f : tile.flags)
            delete f;

        for (auto* m : tile.iceMountains)
            delete m;
    }
}

// ==================================================
// Helper: find tile by (row, col)
// ==================================================
WorldTile* WorldManager::findTile(int row, int col)
{
    for (auto& t : tiles)
    {
        if (t.row == row && t.col == col)
            return &t;
    }
    return nullptr;
}

// ==================================================
// Add tile before generation
// ==================================================
void WorldManager::addTile(int row, int col, TileType type)
{
    WorldTile tile;
    tile.type = type;
    tile.row = row;
    tile.col = col;
    tile.worldPos = glm::vec3(col * tileSize, 0.0f, row * tileSize);

    tiles.push_back(tile);
}

// ==================================================
// START_AREA 相關：只負責地形形狀與水，不放物件
// ==================================================
static void addStartAreaTerrainAndWater(Terrain* t, WorldTile& tile)
{
    // 起始平台
    t->addFlatArea(
        30.0f, 30.0f,   // 中心
        150.0f,         // 半徑
        0.6f,           // 平面高度
        0.9f            // 平滑
    );

    // 插旗
    Flag* flatAreaFlag = new Flag(glm::vec3(
        tile.worldPos.x + 30.0f,
        0.0f,
        tile.worldPos.z + 30.0f
    ));
    flatAreaFlag->color = glm::vec3(1.0f, 0.0f, 0.0f);
    flatAreaFlag->init();
    tile.flags.push_back(flatAreaFlag);

    // 裂縫
    t->addCurvedCrevasse(
        {
            {140,  10},
            {150,  30},
            {150,  52},
            {150, 100},
            {125, 125},
            {115, 150},
            {140, 175},
            {120, 200}
        },
        19.0f,
        25.0f
    );

    t->addCurvedCrevasse(
        {
            {300,  50},
            {260, 100},
            {300, 150},
            {320, 200},
            {310, 290}
        },
        15.0f,
        25.0f
    );

    // 山脊
    t->addRidge(
        {
            {  5,  90},
            { 20,  90},
            { 90, 150},
            {140, 180},
            {200, 160}
        },
        20.0f,
        18.0f
    );

    t->addRidge(
        {
            { 120,  10},
            { 130,  30},
            { 160,  20},
            { 190,  10},
            { 200,  40},
            { 230,  20},
            { 300,  30},
            { 400,  50},
            { 500,  30},
            { 620,  10}
        },
        50.0f,
        30.0f
    );

    t->addRidge(
        {
            { 10, 410},
            { 50, 420},
            {100, 460},
            {150, 500},
            {200, 550},
            {230, 580},
            {300, 570},
            {400, 550},
            {500, 570},
            {620, 590}
        },
        50.0f,
        50.0f
    );

    t->addRidge(
        {
            {420,  70},
            {500, 100},
            {550,  90},
            {600, 160},
            {620, 200}
        },
        30.0f,
        27.0f
    );

    // 整體坡度
    t->addSlope(
        0.0f, 1.0f,  // dirX, dirZ
        1.0f,        // strength
        0.0f, 0.0f,  // offset
        80.0f,       // radius
        1.0f         // smoothness
    );

    // 水：湖 + 裂縫 + 河流
    tile.waterManager = new WaterManager();

    tile.waterManager->addEllipseLake(
        t,
        300.0f, 270.0f,
        108.0f, 56.0f,
        -0.5f
    );

    tile.waterManager->addCrevasseWater(
        t,
        400.0f, 400.0f,
        8.0f,
        10.0f
    );

    std::vector<glm::vec2> riverPts1 = {
        {140,  10},
        {150,  30},
        {150,  52},
        {150, 100},
        {125, 125},
        {115, 150},
        {140, 175},
        {120, 200}
    };

    std::vector<glm::vec2> riverPts2 = {
        {300,  50},
        {260, 100},
        {300, 150},
        {320, 200},
        {310, 290}
    };

    tile.waterManager->addRiver(t, riverPts1, 19.0f * 0.6f, -0.5f);
    tile.waterManager->addRiver(t, riverPts2, 15.0f * 0.6f, -0.5f);

    tile.waterManager->generateAll(*t);
}

// ==================================================
// Create terrain based on tile type
// ==================================================
Terrain* WorldManager::createTerrainFor(TileType type, WorldTile& tile)
{
    switch (type)
    {
        case START_AREA:
        {
            Terrain* t = new Terrain(tile.col, tile.row, tileSize, 7);
            addStartAreaTerrainAndWater(t, tile);
            return t;
        }

        case MOUNTAIN:
        {
            Terrain* t = new Terrain(tile.col, tile.row, tileSize, 10);
            return t;
        }

        case FLOATING_ICE:
            return nullptr;

        case BIG_ICE:
            return nullptr;

        default:
            break;
    }

    return nullptr;
}

// ==================================================
// 物件配置：Start Area 專用
// ==================================================
void WorldManager::setupStartAreaObjects(WorldTile& tile)
{
    // 帳篷（三頂）
    auto addTent = [&](float ox, float oy, float oz, float rotY)
    {
        ObjObject* tent = new ObjObject("assets/tent/Camping_Tent.obj");
        tent->position = glm::vec3(tile.worldPos.x + ox,
                                   oy,
                                   tile.worldPos.z + oz);
        tent->scale = glm::vec3(3.6f);
        tent->rotation.y = rotY;
        tent->color = glm::vec3(1.0f);
        objObjects.push_back(tent);
    };

    addTent(25.0f, 0.8f, 25.0f,  45.0f);
    addTent(45.0f, 0.8f, 15.0f,  15.0f);
    addTent(15.0f, 0.8f, 70.0f, 130.0f);

    // 營火
    {
        ObjObject* campfire = new ObjObject("assets/campfire/Campfire.obj");
        campfire->position = glm::vec3(
            tile.worldPos.x + 40.0f,
            1.0f,
            tile.worldPos.z + 40.0f
        );
        campfire->scale = glm::vec3(0.05f);
        campfire->rotation.y = 180.0f;
        campfire->color = glm::vec3(1.0f);
        objObjects.push_back(campfire);
    }

    // 梯子 / 平台
    {
        ObjObject* ladder = new ObjObject("assets/ladder/staircase.obj");
        ladder->position = glm::vec3(
            tile.worldPos.x + 300.0f,
            2.8f,
            tile.worldPos.z + 150.0f
        );
        ladder->scale = glm::vec3(1.2f);
        ladder->rotation.y = 100.0f;
        ladder->rotation.x = 0.0f;
        ladder->color = glm::vec3(1.0f);
        objObjects.push_back(ladder);
    }

    {
        ObjObject* ladder = new ObjObject("assets/ladder2/Flooring.obj");
        ladder->position = glm::vec3(
            tile.worldPos.x + 140.0f,
            2.8f,
            tile.worldPos.z + 95.0f
        );
        ladder->scale = glm::vec3(1.5f);
        ladder->rotation.y = 70.0f;
        ladder->rotation.x = 0.0f;
        ladder->color = glm::vec3(1.0f);
        objObjects.push_back(ladder);
    }

    // 樹
    auto addTree = [&](float ox, float oz, float rotY, float scale = 0.02f)
    {
        ObjObject* tree = new ObjObject("assets/tree/t1.obj");
        tree->position = glm::vec3(
            tile.worldPos.x + ox,
            -2.0f,
            tile.worldPos.z + oz
        );
        tree->scale = glm::vec3(scale);
        tree->rotation.y = rotY;
        tree->color = glm::vec3(1.0f);
        objObjects.push_back(tree);
    };

    addTree(10.0f,  50.0f, 70.0f, 0.018f);
    addTree( 5.0f,   0.0f,  0.0f);
    addTree( 5.0f,  30.0f,  0.0f);
    addTree( 5.0f, 110.0f, 90.0f);
    addTree( 5.0f, 150.0f, 50.0f);
    addTree( 5.0f, 200.0f, 10.0f);
    addTree( 5.0f, 250.0f, 10.0f);
    addTree( 5.0f, 300.0f, 10.0f);
    addTree(10.0f, 380.0f, 50.0f);

    // 棧橋
    auto addPier = [&](float oy)
    {
        ObjObject* pier = new ObjObject("assets/pier/WoodenBridge_01.obj");
        pier->position = glm::vec3(
            tile.worldPos.x + 610.0f,
            oy,
            tile.worldPos.z + (oy == -1.8f ? 300.0f : 400.0f)
        );
        pier->scale = glm::vec3(0.1f);
        pier->rotation.y = -10.0f;
        pier->color = glm::vec3(1.0f);
        objObjects.push_back(pier);
    };

    addPier(-1.8f);
    addPier(-1.5f);

    std::cout << "OBJ count = " << objObjects.size() << "\n";
}

// ==================================================
// 物件配置：大冰山浮冰 Tile
// ==================================================
void WorldManager::setupBigIceMountains(WorldTile& tile)
{
    // 第一座冰山
    glm::vec3 center1 =
        tile.worldPos + glm::vec3(tileSize * 0.8f, -0.3f, tileSize * 0.3f);

    IceMountain* m1 = new IceMountain(
        center1,
        tileSize * 0.15f,
        10.0f,
        NORTH,
        SOUTH
    );
    m1->rotationY = 110.0f;
    m1->generateMesh();
    tile.iceMountains.push_back(m1);

    // 第二座冰山
    glm::vec3 center2 =
        tile.worldPos + glm::vec3(tileSize * 0.3f, -0.3f, tileSize * 0.8f);

    IceMountain* m2 = new IceMountain(
        center2,
        tileSize * 0.12f,
        7.0f,
        NORTH,
        SOUTH
    );
    m2->rotationY = 30.0f;
    m2->generateMesh();
    tile.iceMountains.push_back(m2);
}

// ==================================================
// 物件配置：浮冰 Tile（含水面 + 小浮冰）
// ==================================================
void WorldManager::setupFloatingWaterAndIce(WorldTile& tile)
{
    // 生成水面 + 同步鄰居 water 邊界
    tile.water = new Water(tile.col, tile.row, tileSize);

    auto northShape = tile.water->getNorthShape();
    auto southShape = tile.water->getSouthShape();
    auto westShape  = tile.water->getWestShape();
    auto eastShape  = tile.water->getEastShape();

    if (tile.terrain)
    {
        tile.terrain->setWaterNorth(northShape);
        tile.terrain->setWaterSouth(southShape);
        tile.terrain->setWaterWest (westShape);
        tile.terrain->setWaterEast (eastShape);
        tile.terrain->generateTerrain();
    }

    if (tile.north)
    {
        tile.north->setWaterSouth(northShape);
        tile.north->generateTerrain();
    }
    if (tile.south)
    {
        tile.south->setWaterNorth(southShape);
        tile.south->generateTerrain();
    }
    if (tile.west)
    {
        tile.west->setWaterEast(westShape);
        tile.west->generateTerrain();
    }
    if (tile.east)
    {
        tile.east->setWaterWest(eastShape);
        tile.east->generateTerrain();
    }

    tile.water->generateMesh();

    // 生成小浮冰
    const int count = 100;

    glm::vec3 c1(0.0f);
    glm::vec3 c2(0.0f);
    float r1 = tileSize * 0.20f;
    float r2 = tileSize * 0.15f;

    if (!tile.iceMountains.empty())
        c1 = tile.iceMountains[0]->position;
    if (tile.iceMountains.size() >= 2)
        c2 = tile.iceMountains[1]->position;

    for (int i = 0; i < count; ++i)
    {
        float rx = static_cast<float>(rand() % static_cast<int>(tileSize));
        float rz = static_cast<float>(rand() % static_cast<int>(tileSize));

        float wx = tile.worldPos.x + rx;
        float wz = tile.worldPos.z + rz;

        glm::vec3 pos(wx, 0.0f, wz);

        // 避開大冰山底部
        if (tile.type == BIG_ICE_MOUNTAIN_FLOATING)
        {
            bool near1 = glm::distance(pos, c1) < r1;
            bool near2 = glm::distance(pos, c2) < r2;
            if (near1 || near2)
                continue;
        }

        IceCube ice;
        ice.pos = pos;

        float h = tile.terrain ? tile.terrain->getHeight(wx, wz) : 0.0f;
        ice.pos.y = h + 0.5f;

        ice.scale = glm::vec3(
            6.0f + (rand() % 50) / 50.0f,
            0.9f,
            6.0f + (rand() % 50) / 50.0f
        );

        ice.mesh = new Cube(true);
        tile.iceCubes.push_back(ice);
    }
}

// ==================================================
// Place objects for each tile（對外：只呼叫這個）
// ==================================================
void WorldManager::placeObjectsFor(WorldTile& tile)
{
    if (tile.type == START_AREA)
        setupStartAreaObjects(tile);

    bool isFloating = (tile.type == FLOATING_ICE ||
                       tile.type == BIG_ICE_MOUNTAIN_FLOATING);

    if (tile.type == BIG_ICE_MOUNTAIN_FLOATING)
        setupBigIceMountains(tile);

    if (isFloating)
        setupFloatingWaterAndIce(tile);
}

// ==================================================
// Generate world
// ==================================================
void WorldManager::generate()
{
    // 1. 建立各 tile 的 Terrain
    for (auto& tile : tiles)
        tile.terrain = createTerrainFor(tile.type, tile);

    // 2. 連結鄰居 Terrain*
    for (auto& tile : tiles)
    {
        if (WorldTile* n = findTile(tile.row - 1, tile.col))
            tile.north = n->terrain;
        else
            tile.north = nullptr;

        if (WorldTile* s = findTile(tile.row + 1, tile.col))
            tile.south = s->terrain;
        else
            tile.south = nullptr;

        if (WorldTile* w = findTile(tile.row, tile.col - 1))
            tile.west = w->terrain;
        else
            tile.west = nullptr;

        if (WorldTile* e = findTile(tile.row, tile.col + 1))
            tile.east = e->terrain;
        else
            tile.east = nullptr;
    }

    // 3. 擺放物件
    for (auto& tile : tiles)
        placeObjectsFor(tile);

}

// ==================================================
// Height query
// ==================================================
float WorldManager::getHeight(float worldX, float worldZ)
{
    for (auto& tile : tiles)
    {
        float x = tile.worldPos.x;
        float z = tile.worldPos.z;

        if (worldX >= x && worldX <= x + tileSize &&
            worldZ >= z && worldZ <= z + tileSize)
        {
            if (tile.terrain)
                return tile.terrain->getHeight(worldX, worldZ);
        }
    }
    return 0.0f;
}

// ==================================================
// Global textures setup
// ==================================================
void WorldManager::setGlobalTextures(GLuint terrainTex0, GLuint iceTex1, GLuint snowTex2)
{
    m_terrainTex0 = terrainTex0;
    m_iceTex1     = iceTex1;
    m_snowTex2    = snowTex2;
}

// ==================================================
// Render
// ==================================================
void WorldManager::render(
    Shader& terrainShader,
    Shader& cubeShader,
    Shader& waterShader,
    Shader& icemountainShader,
    Shader& flagShader,
    Shader& waterSurfaceShader,
    Shader& fogPostShader,
    Shader& smallObjectShader,
    Shader& humanShader,
    Shader& auroraShader,
    Player& player,
    Camera& camera,
    float time,
    const glm::vec3& cameraPos,
    const glm::mat4& view,
    const glm::mat4& projection
)
{
    // 初始化 fog 後處理 FBO（只做一次）
    if (!fogReady)
    {
        fogPP.init(1000, 800);
        fogPP.setMapBounds(glm::vec2(0.0f, 0.0f),
                           glm::vec2(1800.0f, 600.0f));
        fogReady = true;
    }

    auto rebindGlobalTextures = [&]()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_terrainTex0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_iceTex1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_snowTex2);

        glActiveTexture(GL_TEXTURE0);
    };

    // =========================================================
    // Pass A：把場景畫到 FBO（顏色 + 深度）
    // =========================================================
    fogPP.beginScene();
    rebindGlobalTextures();

    glClearColor(0.07f, 0.13f, 0.24f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // --- Terrain pass ---
    for (auto& tile : tiles)
    {
        if (!tile.terrain) continue;

        rebindGlobalTextures();

        terrainShader.use();
        glm::mat4 model(1.0f);
        terrainShader.setMat4("model", model);
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("projection", projection);
        terrainShader.setFloat("minHeight", 0.0f);
        terrainShader.setFloat("maxHeight", tile.terrain->getMaxHeight());

        tile.terrain->draw();
    }

    // --- 小浮冰 Cube ---
    glDisable(GL_CULL_FACE);
    cubeShader.use();
    cubeShader.setMat4("view", view);
    cubeShader.setMat4("projection", projection);
    cubeShader.setVec3("cameraPos", cameraPos);
    cubeShader.setVec3("lightPos", glm::vec3(50.0f, 80.0f, 50.0f));

    for (auto& tile : tiles)
    {
        for (auto& ice : tile.iceCubes)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), ice.pos);
            model = glm::scale(model, ice.scale);
            cubeShader.setMat4("model", model);
            ice.mesh->draw();
        }
    }
    glEnable(GL_CULL_FACE);

    // --- 大冰山 ---
    for (auto& tile : tiles)
    {
        for (IceMountain* m : tile.iceMountains)
        {
            if (!m) continue;

            icemountainShader.use();
            glm::mat4 model(1.0f);
            model = glm::translate(model, m->position);
            model = glm::rotate(model, glm::radians(m->rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

            icemountainShader.setMat4("model", model);
            icemountainShader.setMat4("view", view);
            icemountainShader.setMat4("projection", projection);
            icemountainShader.setVec3("cameraPos", camera.Position);
            icemountainShader.setVec3("lightPos", glm::vec3(50.0f, 80.0f, 50.0f));
            icemountainShader.setFloat("snowMinHeight", 9.0f);
            icemountainShader.setFloat("snowMaxHeight", 13.0f);
            icemountainShader.setFloat("snowSmoothness", 0.7f);

            m->draw();
        }
    }

    // --- 水 Volume ---
    for (auto& tile : tiles)
    {
        if (!tile.water) continue;

        waterShader.use();
        glm::mat4 model(1.0f);
        waterShader.setMat4("model", model);
        waterShader.setVec3("lightPos", glm::vec3(50.0f, 80.0f, 50.0f));
        waterShader.setVec3("cameraPos", cameraPos);
        waterShader.setMat4("view", view);
        waterShader.setMat4("projection", projection);
        waterShader.setFloat("time", time);

        tile.water->draw();
    }

    // --- 水面（湖/河） ---
    for (auto& tile : tiles)
    {
        if (!tile.waterManager) continue;

        waterSurfaceShader.use();
        glm::mat4 model(1.0f);
        waterSurfaceShader.setMat4("model", model);
        waterSurfaceShader.setMat4("view", view);
        waterSurfaceShader.setMat4("projection", projection);
        waterSurfaceShader.setFloat("time", time);

        tile.waterManager->draw();
    }

    // --- 旗幟 ---
    for (auto& tile : tiles)
    {
        for (Flag* f : tile.flags)
            f->draw(flagShader);
    }

    // --- smallObjects / objObjects：⚠️ 只畫一次，不要放在 tile 迴圈裡 ---
    glDisable(GL_CULL_FACE);
    smallObjectShader.use();
    smallObjectShader.setMat4("view", view);
    smallObjectShader.setMat4("projection", projection);

    for (auto* obj : smallObjects)
        obj->draw(smallObjectShader);

    for (auto* obj : objObjects)
        obj->draw(smallObjectShader);
    glEnable(GL_CULL_FACE);

    // --- Player / Human ---
    glDisable(GL_CULL_FACE);
    humanShader.use();
    humanShader.setMat4("view", view);
    humanShader.setMat4("projection", projection);
    humanShader.setVec3("cameraPos", cameraPos);
    humanShader.setVec3("lightPos", glm::vec3(50.0f, 80.0f, 50.0f));
    player.Draw(humanShader, time);
    glEnable(GL_CULL_FACE);

    // =========================================================
    // Pass B：fog 後處理 + 極光，輸出到螢幕
    // =========================================================
    fogPP.renderToScreen(
        fogPostShader,
        auroraShader,
        view,
        projection,
        cameraPos,
        static_cast<float>(glfwGetTime())
    );

    // 還原狀態
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glActiveTexture(GL_TEXTURE0);
}
