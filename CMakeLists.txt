cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
project(gbc)

if(APPLE)
    set(HOMEBREW_PATH /opt/homebrew)
    # Set up SDL2
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
elseif(WIN32)
    # https://stackoverflow.com/questions/60020392/why-is-timespec-get-not-supported-by-mingw-gcc-8-2-0-std-c11
    set(CMAKE_C_FLAGS "-D_UCRT")
    set(CMAKE_CXX_FLAGS "-D_UCRT")
    set(MSYS2_PATH "C:\\MyPrograms\\msys2")

endif()

# Set up the source files
set(SOURCES
    gbc.c
    cpu.c
    mbc.c
    cartridge.c
    memory.c
    audio.c
    graphic.c
    io.c
    timer.c
    utils.c
    instruction_set.c
    main.c
)

# imgui
set(IMGUI_SOURCES
    gui/main_sdl2.cpp
    gui/mywindow.cpp
    gui/imgui/imgui.cpp
    gui/imgui/imgui_draw.cpp
    gui/imgui/imgui_widgets.cpp
    gui/imgui/imgui_tables.cpp
    gui/imgui/imgui_demo.cpp
    gui/imgui/backends/imgui_impl_sdl2.cpp
    gui/imgui/backends/imgui_impl_opengl3.cpp
    gui/nativefiledialog/src/nfd_common.c
)

set(IMGUI_INCLUDE_DIRS ./ gui/ gui/imgui gui/imgui/backends gui/nativefiledialog/src/include)

if (WIN32)
    list(APPEND IMGUI_INCLUDE_DIRS
        ${MSYS2_PATH}/mingw64/include/SDL2
    )
    list(APPEND IMGUI_SOURCES
        gui/nativefiledialog/src/nfd_win.cpp)
    link_directories(
        ${MSYS2_PATH}/mingw64/lib
    )
elseif(APPLE)
    list(APPEND IMGUI_INCLUDE_DIRS
        ${HOMEBREW_PATH}/include
    )

    list(APPEND IMGUI_SOURCES
        gui/nativefiledialog/src/nfd_cocoa.m)
    link_directories(
        ${HOMEBREW_PATH}/lib
    )
elseif(LINUX)
    list(APPEND IMGUI_SOURCES
        gui/nativefiledialog/src/nfd_gtk.c)
endif()

if (WIN32)
    SET(IMGUI_LIBS sdl2 gdi32 opengl32 imm32 ucrt pthread)
elseif(APPLE)
    set(IMGUI_LIBS "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" ${SDL2_LIBRARIES})
endif()

add_compile_options(-g)
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

include_directories(${IMGUI_INCLUDE_DIRS})
add_executable(xgbc ${SOURCES} ${IMGUI_SOURCES})
target_link_libraries(xgbc ${IMGUI_LIBS})