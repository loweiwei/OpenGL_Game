# 檔案用途整理

這份文件整理主要檔案在做什麼，方便之後回頭看或跟別人介紹專案。

## 根目錄

| 檔案 | 用途 |
| --- | --- |
| `CMakeLists.txt` | CMake 建置設定，連結 GLFW、GLAD、GLM，build 後複製 `assets/` 和 `shaders/`。 |
| `vcpkg.json` | vcpkg manifest，記錄專案需要的第三方套件。 |
| `README.md` | 專案簡介、操作方式、快速建置與文件連結。 |
| `note.txt` | 開發時暫存的地形和水面想法筆記。 |

## src 核心流程

| 檔案 | 用途 |
| --- | --- |
| `main.cpp` | 程式進入點。初始化 GLFW / GLAD、建立 window、載入 shader / texture、建立 world / player / camera，並執行主迴圈。 |
| `Shader.h`, `Shader.cpp` | 封裝 shader 編譯、連結和 uniform 設定。 |
| `Camera.h`, `Camera.cpp` | 第三人稱相機，處理滑鼠旋轉、滾輪縮放和跟隨玩家。 |

## 世界與地形

| 檔案 | 用途 |
| --- | --- |
| `WorldManager.hpp`, `WorldManager.cpp` | 管理 tile、地形、水、冰山、旗子、OBJ 物件，也負責主要 render 順序。 |
| `Terrain.hpp`, `Terrain.cpp` | 產生 terrain height map 和 mesh。支援 Perlin noise、平地、斜坡、裂縫、山脊，也提供 `getHeight()`。 |
| `utils/PerlinNoise.hpp` | Perlin noise 實作，用來產生地形基礎高度。 |
| `IceMountain.hpp`, `IceMountain.cpp` | 產生大型冰山 mesh，並用 shader 做冰和雪的材質效果。 |
| `Cube.hpp`, `Cube.cpp` | cube mesh，主要用在小浮冰或簡單幾何物件。 |
| `Flag.hpp`, `Flag.cpp` | 場景中的旗子物件。 |

## 水面

| 檔案 | 用途 |
| --- | --- |
| `Water.hpp`, `Water.cpp` | 大範圍水域 mesh，例如浮冰區底下的水。 |
| `WaterSurface.hpp`, `WaterSurface.cpp` | 貼著地形產生的水面 mesh，例如湖泊、河流、裂縫水。 |
| `WaterManager.hpp`, `WaterManager.cpp` | 統一管理多個 `WaterSurface`，提供 `addLake()`、`addRiver()`、`addEllipseLake()` 等建立方式。 |

## 玩家、動畫、碰撞

| 檔案 | 用途 |
| --- | --- |
| `Player.hpp`, `Player.cpp` | 玩家位置、速度、重力、跳躍、OBB 更新和移動碰撞。實際外觀交給 `RobotModel` 畫。 |
| `RobotModel.hpp`, `RobotModel.cpp` | 階層式機器人模型。用 cube 組出身體和四肢，支援關節控制、walk、run、bow、衛星動畫。 |
| `OBB.hpp` | OBB 資料結構，記錄中心、half size 和 orientation。 |
| `Collision.hpp` | OBB vs OBB 的 SAT 碰撞函式，還有一些 debug / OBB 輔助函式。 |

## OBJ 模型與小物件

| 檔案 | 用途 |
| --- | --- |
| `ObjMesh.hpp`, `ObjMesh.cpp` | 使用 tinyobjloader 載入 OBJ / MTL，使用 stb_image 載入貼圖，建立 VAO / VBO / EBO。 |
| `ObjObject.hpp` | 包住 `ObjMesh`，加上 position、rotation、scale、color 和 OBB。 |
| `SmallObject.hpp` | 小型物件資料結構，目前主要保留給簡單場景物件使用。 |
| `tiny_obj_loader.h` | 第三方 OBJ loader。 |
| `stb_image.h` | 第三方圖片載入 header。 |

## 後處理與極光

| 檔案 | 用途 |
| --- | --- |
| `FogPostProcess.hpp`, `FogPostProcess.cpp` | 建立 framebuffer、color texture、depth texture，最後用 fullscreen quad 做霧效和極光 pass。 |
| `Aurora.h`, `Aurora.cpp` | 原本的極光平面繪製類別。目前主要極光效果是在 post-process 中疊加。 |

## shaders

| Shader | 用途 |
| --- | --- |
| `terrain.vert`, `terrain.frag` | 地形渲染，依高度和貼圖產生雪地效果。 |
| `water.vert`, `water.frag` | 大範圍水域渲染。 |
| `water_surface.vert`, `water_surface.frag` | 湖泊、河流、裂縫水面。 |
| `IceMountain.vert`, `IceMountain.frag` | 冰山渲染，包含冰和雪的視覺效果。 |
| `cube.vert`, `cube.frag` | cube / 小浮冰渲染。 |
| `small_object.vert`, `small_object.frag` | OBJ 場景物件渲染。 |
| `human.vert`, `human.frag` | 機器人角色渲染。 |
| `flag.vert`, `flag.frag` | 旗子渲染。 |
| `fog_post.vert`, `fog_post.frag` | fullscreen post-process 霧效。 |
| `aurora.vert`, `aurora.frag` | 極光效果。 |
| `basic.vert`, `basic.frag` | 基本 shader，目前偏向測試或簡單物件使用。 |
