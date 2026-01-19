cmake_minimum_required(VERSION 3.16)
project(OpenGL_Game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------
# VCPKG (GLFW / GLAD / GLM)
# --------------------------------------------------
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# --------------------------------------------------
# Source files
# --------------------------------------------------
file(GLOB SRC_FILES
    "src/*.cpp"
    "src/*.h"
)

add_executable(OpenGL_Game ${SRC_FILES})

# --------------------------------------------------
# Include directories
# --------------------------------------------------
target_include_directories(OpenGL_Game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --------------------------------------------------
# Link libraries
# --------------------------------------------------
target_link_libraries(OpenGL_Game
    glfw
    glad::glad
    glm::glm
)

# ===========================================
# 自動複製 assets/ & shaders/ 到 Debug 資料夾
# ===========================================

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
