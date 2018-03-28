cmake_minimum_required(VERSION 3.9)

project(MyCraft_Demo C)

set(CMAKE_C_STANDARD 11)

if (WIN32 AND MINGW)
    # Workaround for no references in GLEW
    add_definitions(-DGLEW_STATIC)

    # -mwindows: window subsystem
    # -mconsole: console subsystem
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows -mconsole -static")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__USE_MINGW_ANSI_STDIO=1")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

if (${CMAKE_BUILD_TYPE} EQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og")
elseif(${CMAKE_BUILD_TYPE} EQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS -Ofast")
endif()

find_package(PkgConfig REQUIRED)
pkg_search_module(GLEW REQUIRED glew)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(
        ${GLEW_INCLUDE_DIRS}
        ${GLFW_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

set(SOURCE_FILES
        src/main.c
        src/debug.h
        src/utils.c
        src/utils.h
        src/glutils.c
        src/glutils.h
        src/block.c
        src/block.h
        src/model.c
        src/model.h
        src/player.c
        src/player.h
        src/chunk.c
        src/chunk.h)

set(LIB_SOURCE_FILES
        lib/lodepng.c
        lib/png_loader.c
        lib/png_loader.h)

set(APPRES_OBJS)
if (WIN32 AND MINGW)
    set(WINRES_OUT ${CMAKE_BINARY_DIR}/appres.o)

    add_custom_command(
            OUTPUT ${WINRES_OUT}
            COMMAND windres ${CMAKE_CURRENT_SOURCE_DIR}/appres.rc ${WINRES_OUT}
    )

    add_custom_target(WinResObj ALL DEPENDS ${WINRES_OUT})

    set(APPRES_OBJS ${WINRES_OUT})
endif()

add_executable(MyCraft_Demo ${SOURCE_FILES} ${LIB_SOURCE_FILES} ${APPRES_OBJS})

target_link_libraries(MyCraft_Demo
        ${GLEW_LIBRARIES}
        ${GLFW_LIBRARIES}
#        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcglm.dll.a
#        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcglm.a
        )

if (WIN32 AND MINGW)
    target_link_libraries(MyCraft_Demo
            # -lGL
            opengl32
            # -lmingw32
            # mingw32
            # -lmingwex
            mingwex
            # -lmsvcrt
            # msvcrt
            # -lmsvcr120
            # msvcr120
            # -lucrtbase
            ucrtbase
            )
endif()

message(STATUS "CMake Config: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: " ${CMAKE_C_COMPILER})
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "External Object: ${APPRES_OBJS}")
message(STATUS "External library: " ${GLEW_LIBRARIES} " " ${GLFW_LIBRARIES})
