# Snow Field OpenGL Game

這是我用 C++ 和 OpenGL 做的雪地第三人稱場景。主要目標不是做完整商業遊戲，而是把圖學課程中學到的內容整合成一個可以操作、可以展示的 3D 場景。

目前專案裡有地形生成、水面、霧效、極光、OBJ 模型載入、角色移動、碰撞，以及一個可以控制關節的機器人角色。

## 專案內容

- 使用 OpenGL 3.3 Core Profile 繪製 3D 場景。
- 使用 Perlin noise 產生雪地高度地形。
- 額外用 mask 做出平地、山脊、斜坡和裂縫，不只靠純 noise。
- 場景用 tile 組合，目前包含起始雪地、浮冰區和山地區。
- 水面分成湖泊、河流、裂縫水面和大範圍水域。
- 玩家使用第三人稱視角，可以移動、跳躍和旋轉鏡頭。
- 場景物件使用 OBJ 載入，例如帳篷、營火、樹、橋和梯子。
- 玩家和場景物件使用 OBB 做碰撞處理。
- 機器人角色是用階層式 transform 組出來的，可以切換走路、跑步和鞠躬動作。
- 最後用 framebuffer 做霧效後處理，並在畫面上疊加極光效果。

## 文件

- [模組串接與 Render Pipeline](docs/architecture.md)
- [建置與執行方式](docs/build-run.md)
- [檔案用途整理](docs/file-guide.md)

## 使用技術

- C++17
- OpenGL 3.3
- GLFW
- GLAD
- GLM
- GLSL shader
- CMake
- vcpkg
- tinyobjloader
- stb_image

## 操作方式

- `W A S D`：移動角色。
- `Space`：跳躍。
- 滑鼠左鍵拖曳：旋轉第三人稱相機。
- 滑鼠滾輪：調整相機距離。
- 滑鼠右鍵：切換 console 說明選單。
- `1`：停止機器人動作。
- `2`：走路。
- `3`：跑步。
- `4`：鞠躬。
- `.` / `,`：切換目前控制的機器人關節。
- `x/X`、`y/Y`、`z/Z`：旋轉目前選到的關節。
- `r`：開關衛星公轉。
- `t`：開關衛星自轉。
- `u`、`v`、`w`：切換衛星旋轉軸。

## 快速建置

需要先安裝：

- Visual Studio 2022 C++ tools
- CMake 3.16 以上
- vcpkg

安裝依賴：

```powershell
vcpkg install --triplet x64-windows
```

建置：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Debug
```

執行：

```powershell
.\build\Debug\OpenGL_Game.exe
```

`assets/` 和 `shaders/` 會在 build 後複製到執行檔旁邊。

如果要看比較完整的建置說明和常見問題，可以看 [docs/build-run.md](docs/build-run.md)。

## 可以展示的部分

- 從起始區域移動到雪地和浮冰區。
- 看 terrain 的高低變化、裂縫和湖泊。
- 展示水面、霧效和極光。
- 移動角色並跳上場景物件，展示 OBB 碰撞。
- 切換機器人走路、跑步、鞠躬。
- 手動切換關節並旋轉，展示階層式模型控制。

## 後續可以整理的地方

- 補上截圖或展示影片。
- 在乾淨電腦上測試一次完整建置流程。
- 把 debug 用的 marker 關掉，只留下正式展示畫面。
- 如果要繼續擴充，可以加任務目標或收集物件，讓它更像完整遊戲。
