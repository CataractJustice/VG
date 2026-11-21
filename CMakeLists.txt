cmake_minimum_required(VERSION 3.22.1)

project(VG)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-volatile -g3 -fpermissive")
if(NOT CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

file(GLOB_RECURSE SOURCES GameAssets/*.cpp)

add_subdirectory(./EngineLink/libs/glfw)
add_subdirectory(./EngineLink/libs/freetype2)
add_subdirectory(./EngineLink/libs/enet)

set(INCLUDE_DIRS
PUBLIC ./EngineLink/ 
PUBLIC ./EngineLink/libs 
PUBLIC ./EngineLink/libs/imgui 
PUBLIC ./EngineLink/libs/glfw/include 
PUBLIC ./EngineLink/libs/glew-2.2.0/include
PUBLIC ./EngineLink/libs/freetype2/include
PUBLIC ./EngineLink/libs/enet/include
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})


if(MINGW)
target_link_options(${PROJECT_NAME} PRIVATE
"-Wl,--allow-shlib-undefined"
"-Wl,--unresolved-symbols=ignore-all"
)
endif()

target_include_directories(${PROJECT_NAME} ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ./GameAssets/Code/)

if(MINGW)
target_link_directories(${PROJECT_NAME} PUBLIC ./EngineLink/build-mingw)
else()
target_link_directories(${PROJECT_NAME} PUBLIC ./EngineLink/build-linux)
endif()

set(LINK_DIRS
PRIVATE ./EngineLink/libs/freetype2
PRIVATE ./EngineLink/libs/enet
)

if(MINGW)
target_link_libraries(${PROJECT_NAME}
    PRIVATE "${CMAKE_SOURCE_DIR}/EngineLink/libs/glew-2.2.0/lib-win/libglew32.dll.a"
)
else()
set(LINK_LIBS ${LINK_LIBS} 
PRIVATE ./EngineLink/libs/glew-2.2.0/lib
)
endif()
target_link_directories(${PROJECT_NAME} ${LINK_DIRS})

find_package(OpenGL REQUIRED)
set(LINK_LIBS 
PRIVATE glfw
PRIVATE Kirino
PRIVATE TealEngine
PRIVATE OpenGL::GL
)
if(MINGW)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--stack,16777216)
else()
set(LINK_LIBS ${LINK_LIBS} PUBLIC GLEW)
endif()

target_link_libraries(${PROJECT_NAME} 
${LINK_LIBS}
)
