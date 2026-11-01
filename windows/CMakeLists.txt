cmake_minimum_required(VERSION 3.20)
project(ScreenDimmerWindows LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_executable(ScreenDimmer WIN32 ScreenDimmer.cpp)
    target_compile_definitions(ScreenDimmer PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX WINVER=0x0A00 _WIN32_WINNT=0x0A00)
    target_link_libraries(ScreenDimmer PRIVATE comctl32 shell32 ole32 user32 gdi32)
    if(MSVC)
        target_compile_options(ScreenDimmer PRIVATE /W4 /permissive- /utf-8)
    endif()
endif()

enable_testing()
add_executable(screen_dimmer_logic_test tests/logic_test.cpp)
add_test(NAME logic COMMAND screen_dimmer_logic_test)
