#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "ObjObject.hpp" 
#include "SmallObject.hpp"
#include "Terrain.hpp"
#include "Cube.hpp"
#include "Water.hpp"
#include "Shader.h"
#include "IceMountain.hpp"
#include "Flag.hpp"
#include "WaterManager.hpp"
#include "FogWall.hpp"
#include "Player.hpp"
#include "tent.hpp"
#include "Collision.hpp"
#include "FogPostProcess.hpp"
#include "Camera.h"

enum TileType {
    START_AREA,
    FLOATING_ICE,
    BIG_ICE,
    MOUNTAIN,
    BIG_ICE_MOUNTAIN_FLOATING
};

struct IceCube {
    glm::vec3 pos;
    glm::vec3 scale;
    Cube* mesh;
};

struct WorldTile {
    TileType type;
    int row, col;
    glm::vec3 worldPos;

    Terrain* terrain = nullptr;
    Water* water = nullptr;
    WaterManager* waterManager = nullptr;

    std::vector<IceCube> iceCubes;
    std::vector<Flag*> flags;
    std::vector<IceMountain*> iceMountains;
    

    Terrain* north = nullptr;
    Terrain* south = nullptr;
    Terrain* west  = nullptr;
    Terrain* east  = nullptr;
};
class Camera;
class WorldManager
{
public:
    WorldManager(float tileSize);
    ~WorldManager();

    void addTile(int row, int col, TileType type);
    void generate();
    void setGlobalTextures(GLuint terrainTex0, GLuint iceTex1, GLuint snowTex2);
    std::vector<SmallObject*> smallObjects;
    std::vector<ObjObject*> getObjects() { return objObjects; }

    FogPostProcess fogPP;
    
    

    bool fogReady = false;
    void render(
        Shader &terrainShader,
        Shader &cubeShader,
        Shader &waterShader,
        Shader &icemountainShader,
        Shader &flagShader,
        Shader &waterSurfaceShader,
        Shader &fogPostShader,      
        Shader &smallObjectShader,
        Shader &humanShader,
        Shader &auroraShader,     
        Player &player,
        Camera &camera,
        float time,          
        const glm::vec3& cameraPos,
        const glm::mat4& view,      
        const glm::mat4& projection 
    );
    
    float getHeight(float worldX, float worldZ);

private:
    GLuint m_terrainTex0 = 0;
    GLuint m_iceTex1     = 0;
    GLuint m_snowTex2    = 0;

    float tileSize;
    std::vector<WorldTile> tiles;
    std::vector<FogWall> fogWalls;
    std::vector<ObjObject*> objObjects;


    WorldTile* findTile(int row, int col);
    Terrain* createTerrainFor(TileType type, WorldTile& tile);
    
    void setupStartAreaObjects(WorldTile& tile);
    void setupBigIceMountains(WorldTile& tile);
    void setupFloatingWaterAndIce(WorldTile& tile);

    void placeObjectsFor(WorldTile& tile);
    void addFogBoundary();
};
