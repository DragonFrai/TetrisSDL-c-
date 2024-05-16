cmake_minimum_required(VERSION 3.27)
project(TetrisSDL)

set(CMAKE_CXX_STANDARD 17)

# =====================
# DEPENDENCIES INCLUDES

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# DEPENDENCIES INCLUDES
# =====================
# PROJECT SRC FILES

add_executable(TetrisSDL src/main.cpp)

# /PROJECT SRC FILES
# ==================
# DEPENDENCIES LINKS

target_link_libraries(TetrisSDL ${SDL2_LIBRARIES})

# DEPENDENCIES LINKS
# ==================
