cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-O3 -s")

project(verifySN)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)
 
add_executable(verifySN verifysn.cpp "ui.hpp")

target_link_libraries(verifySN PRIVATE
    ftxui::component
    ftxui::dom
    ftxui::screen
)
