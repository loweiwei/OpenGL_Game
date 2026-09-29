# 建置與執行方式

這份文件整理我目前專案的建置方式。因為這是 Windows + OpenGL + vcpkg 的專案，最容易出問題的是 CMake、Visual Studio 和 vcpkg 路徑。

## 需要的工具

- Windows
- Visual Studio 2022
- Visual Studio 的 Desktop development with C++
- CMake 3.16 以上
- vcpkg

## 安裝依賴

專案有 `vcpkg.json`，所以依賴套件不用一個一個寫在 CMake 裡。目前需要：

- glfw3
- glad
- glm

在專案根目錄執行：

```powershell
vcpkg install --triplet x64-windows
```

如果使用 manifest mode，vcpkg 會讀 `vcpkg.json`。

## 設定 VCPKG_ROOT

如果系統還沒有設定 `VCPKG_ROOT`，可以先在 PowerShell 設定：

```powershell
$env:VCPKG_ROOT="D:/dev/vcpkg/vcpkg"
```

路徑要換成自己電腦上的 vcpkg 位置。

## Configure

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

這一步會產生 `build/`。如果之前 `build/` 是從不同路徑產生的，建議刪掉 `build/` 後重新 configure。

## Build

```powershell
cmake --build build --config Debug
```

建置後，CMake 會把 `assets/` 和 `shaders/` 複製到執行檔旁邊，這樣程式執行時才能找到模型、貼圖和 shader。

## Run

```powershell
.\build\Debug\OpenGL_Game.exe
```

## VS Code 的情況

`.vscode/tasks.json` 裡原本有寫 Visual Studio 內建 CMake 的完整路徑。如果不同電腦 Visual Studio 安裝位置不一樣，那個路徑可能不能用。

比較穩的方式是：

- 讓 `cmake` 可以在 terminal 直接被找到。
- 或是把 `.vscode/tasks.json` 裡的 CMake 路徑改成自己電腦上的路徑。

## 常見問題

`cmake` 找不到：
代表 CMake 沒有加到 PATH，或 Visual Studio 內建 CMake 的路徑不一樣。

找不到 `glfw3`、`glad`、`glm`：
通常是 `CMAKE_TOOLCHAIN_FILE` 沒有指到 vcpkg 的 `vcpkg.cmake`，或是還沒 `vcpkg install`。

執行時 shader 載入失敗：
確認執行檔旁邊有 `shaders/` 資料夾，或是從正確工作目錄執行。

執行時模型或貼圖載入失敗：
確認執行檔旁邊有 `assets/` 資料夾。

舊 build 不能用：
如果 `build/CMakeCache.txt` 裡記錄的是舊專案路徑，直接刪掉 `build/` 後重跑 configure。
