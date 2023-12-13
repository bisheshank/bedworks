cmake_minimum_required(VERSION 3.16)

# Sets project name
project(yesmansky LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)

# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)
include_directories(libraries)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp

    src/mainwindow.h
    src/realtime.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shaderloader.h
    src/utils/aspectratiowidget/aspectratiowidget.hpp
    src/utils/debug.h

    src/primitives/primitive.h
    src/primitives/primitive.cpp

    src/camera/camera.h
    src/camera/camera.cpp
    src/primitives/sphere.h src/primitives/sphere.cpp
    src/primitives/cube.h src/primitives/cube.cpp
    src/primitives/cylinder.h src/primitives/cylinder.cpp
    src/primitives/cone.h src/primitives/cone.cpp



    src/noise/fastnoise.h src/noise/fastnoise.cpp
    src/meshes/texture.h src/meshes/texture.cpp
    src/meshes/model.h src/meshes/model.cpp
    src/meshes/mesh.h src/meshes/mesh.cpp

    libraries/include/glad/glad.h
    libraries/include/json/json.h
    libraries/include/stb/stb_image.h
    src/utils/ebo.h src/utils/ebo.cpp
    src/utils/vao.h src/utils/vao.cpp
    src/utils/vbo.h src/utils/vbo.cpp
    src/utils/shader.h src/utils/shader.cpp
    src/glad.c
    src/meshes/skybox.h src/meshes/skybox.cpp



)

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC
    glew/src/glew.c)
include_directories(${PROJECT_NAME} PRIVATE glew/include)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
)

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES
    resources/shaders/phong.frag
    resources/shaders/phong.vert
    resources/shaders/framebuffer.vert
    resources/shaders/framebuffer.frag
    resources/shaders/model.frag
    resources/shaders/model.vert
    resources/shaders/instancing.vert
)

target_sources(yesmansky_other_files
  PRIVATE
  resources/shaders/instancing.vert

)

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )
endif()

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()
# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
