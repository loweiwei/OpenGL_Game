# 模組串接與 Render Pipeline

這份文件主要整理專案裡幾個模組是怎麼接在一起的，以及每一幀大概怎麼跑。README 只放簡介，這邊放比較細的流程。

## 整體串接

```text
main.cpp
  建立 window / OpenGL context
  載入 shader、texture
  建立 Player、Camera、WorldManager
  進入 main loop
        |
        v
WorldManager
  建立 tile
  產生 Terrain / Water / IceMountain / Flag / ObjObject
  負責 render 時的場景繪製順序
        |
        +---- Terrain
        |       產生高度圖、terrain mesh、提供 getHeight()
        |
        +---- Water / WaterManager / WaterSurface
        |       產生湖、河、裂縫水面和浮冰區水域
        |
        +---- ObjObject / ObjMesh
        |       載入 OBJ 模型，建立物件 OBB
        |
        +---- FogPostProcess
        |       建立 framebuffer，最後做霧效和極光
        |
        v
Player
  處理位置、速度、重力、跳躍、OBB
        |
        v
RobotModel
  用階層式 transform 畫出機器人和動作
```

## main loop 做的事

`main.cpp` 的主迴圈可以分成 update 和 render 兩大段。

```text
while window is open:
  1. 計算 deltaTime
  2. processInput()
  3. 更新場景物件 OBB
  4. player.applyMovement()
  5. 查地形高度，修正玩家位置
  6. camera.FollowPlayer()
  7. 計算 view / projection
  8. world.render(...)
  9. swap buffers / poll events
```

## Update Pipeline

```text
Input
  WASD 轉成移動方向
  Space 觸發跳躍
  滑鼠控制相機旋轉和距離
        |
        v
Player Movement
  速度乘上 deltaTime
  加上重力
  分 X / Y / Z 軸嘗試移動
        |
        v
Collision
  ObjObject 每幀更新 world OBB
  Player 用 future OBB 和物件做 SAT 碰撞
  水平碰撞會把玩家推出去
  Y 軸會處理平台落地和撞到天花板
        |
        v
Terrain Height
  如果玩家不在平台上，就用 WorldManager::getHeight()
  讓玩家貼在地形表面，不會掉到 terrain 下面
        |
        v
Camera
  TPS 相機跟隨玩家
  如果相機低於地形，也會把相機高度修正上來
```

## Render Pipeline

目前 rendering 是先畫到 framebuffer，再做 post-process。

```text
1. beginScene()
   FogPostProcess 綁定 FBO
   清掉 color / depth

2. Terrain Pass
   每個 tile 的 terrain 依照 height 和 texture 繪製

3. Ice / Mountain Pass
   繪製小浮冰 cube
   繪製大型 IceMountain mesh

4. Water Pass
   繪製大範圍水域 Water
   繪製湖泊、河流、裂縫 WaterSurface

5. Object Pass
   繪製 Flag
   繪製 OBJ 場景物件，例如 tent、tree、campfire、pier、ladder

6. Player Pass
   繪製 Player 的 RobotModel
   RobotModel 會依照目前 action 做 walk / run / bow 動畫

7. Post Process Pass
   FogPostProcess 讀取 scene color texture
   讀取 depth texture，重建世界座標
   套用地圖邊界霧、高度霧
   疊加 aurora shader

8. Present
   輸出到螢幕
```

## 地形和水的關係

地形本身由 `Terrain` 產生，高度主要來自 Perlin noise。為了讓場景比較可控，我另外加了幾種 mask：

- `flatMask`：做起始平台或平坦區。
- `slopeMask`：讓地形往指定方向有坡度。
- `crevasseMask`：壓低地形，形成裂縫。
- `ridgeMask`：抬高地形，形成山脊。

水面不是單純一張固定平面，而是由 `WaterManager` 建立不同形狀的 `WaterSurface`。例如湖泊用橢圓範圍，河流用 polyline 取樣生成一串水面。

## 碰撞處理

場景物件用 `ObjObject` 載入模型後，會先根據模型頂點算出 local OBB。每一幀再依照位置、旋轉、縮放更新成 world OBB。

玩家也有自己的 OBB。移動時不是直接把玩家移到新位置，而是先建立 future OBB，分 X、Y、Z 軸測試碰撞。這樣可以比較容易處理水平碰撞、落到平台上、撞到平台底部這幾種情況。

## 機器人動畫

`RobotModel` 不是載入外部模型，而是用 cube 組出身體、頭、手、腳。每個部位用 parent-child transform 串起來，所以肩膀、手肘、髖關節、膝蓋可以分開旋轉。

目前支援：

- Stop
- Walk
- Run
- Bow
- 手動選關節後用 X / Y / Z 軸旋轉
- 衛星公轉和自轉
