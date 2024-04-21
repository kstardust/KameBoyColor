cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
project(gbc)

if(APPLE)
    set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
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

add_executable(main ${SOURCES})
