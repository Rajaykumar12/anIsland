cmake_minimum_required(VERSION 3.10)
project(OpenGLTerrainInstancing)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find necessary packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Add your source files
set(SOURCES
    main_new.cpp          # Refactored main with modular systems
    src/Terrain.cpp
    src/Camera.cpp
    src/NoiseMap.cpp
    src/Shader.cpp
    src/TreeSystem.cpp
    src/NPCSystem.cpp
    src/ParticleSystem.cpp
    src/WaterSystem.cpp
    src/GrassSystem.cpp
    src/LightingSystem.cpp
    src/RainSystem.cpp
    src/SplashSystem.cpp
    src/CinematicSystem.cpp
    glad.c            # Using the glad.c present at project root
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Tell CMake where your header files are
# include/  -> Terrain.h, Shader.h, Camera.h, NoiseMap.h
# .     -> glad/glad.h (if glad was placed in the root)
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${CMAKE_SOURCE_DIR}   # fallback so #include <glad/glad.h> resolves
)

# Link the libraries to your executable
target_link_libraries(${PROJECT_NAME} PRIVATE 
    OpenGL::GL 
    glfw
)

# Optional: Copy the assets folder to the build directory automatically
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_BINARY_DIR}/assets
)