cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
project(gbc)

if(APPLE)
    set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
    set(HOMEBREW_PATH /opt/homebrew)
elseif(WIN32)
    # https://stackoverflow.com/questions/60020392/why-is-timespec-get-not-supported-by-mingw-gcc-8-2-0-std-c11
    set(CMAKE_C_FLAGS "-D_UCRT")
    set(CMAKE_CXX_FLAGS "-D_UCRT")
    set(MSYS2_PATH "F:\\msys2")
endif()    

# Set up the source files
set(SOURCES
    gbc.c
    cpu.c
    mbc.c
    cartridge.c
    memory.c
    io.c
    utils.c
    instruction_set.c
    main.c)

# imgui
set(IMGUI_SOURCES
    gui/main_gl.cpp
    gui/mywindow.cpp
    gui/imgui/imgui.cpp
    gui/imgui/imgui_draw.cpp
    gui/imgui/imgui_widgets.cpp    
    gui/imgui/imgui_tables.cpp
    gui/imgui/imgui_demo.cpp
    gui/imgui/backends/imgui_impl_glfw.cpp
    gui/imgui/backends/imgui_impl_opengl3.cpp
)    

set(IMGUI_INCLUDE_DIRS gui/ gui/imgui gui/imgui/backends)

if (WIN32)
    list(APPEND IMGUI_INCLUDE_DIRS 
        ${MSYS2_PATH}/mingw64/include
        ${MSYS2_PATH}/ucrt64/include
    )

    link_directories(
        ${MSYS2_PATH}/mingw64/lib
        ${MSYS2_PATH}/ucrt64/lib
    )    
elseif(APPLE)
    list(APPEND IMGUI_INCLUDE_DIRS
        ${HOMEBREW_PATH}/include
    )

    link_directories(        
        ${HOMEBREW_PATH}/lib
    )
endif()

if (WIN32)
    SET(IMGUI_LIBS libglfw3.a gdi32 opengl32 imm32 ucrt)    
elseif(APPLE)
    set(IMGUI_LIBS libglfw.a "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

include_directories(${IMGUI_INCLUDE_DIRS})
add_executable(main ${SOURCES} ${IMGUI_SOURCES})
target_link_libraries(main ${IMGUI_LIBS})