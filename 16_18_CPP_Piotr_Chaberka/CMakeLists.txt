cmake_minimum_required(VERSION 3.10)
project(SolitaireGame)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your source file
add_executable(SolitaireGame 16_18_CPP_Piotr_Chaberka.cpp)

# Force UTF-8 on Windows
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Optional: link statically (if using MinGW)
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()
