cmake_minimum_required(VERSION 3.9)

project(MyCraft_Demo C)

set(CMAKE_C_STANDARD 11)

if (WIN32 AND MINGW)
    # Workaround for no references in GLEW
    add_definitions(-DGLEW_STATIC)

    # -mwindows: window subsystem
    # -mconsole: console subsystem
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows -mconsole -static")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLEW REQUIRED glew)
pkg_search_module(GLFW REQUIRED glfw3)
#pkg_search_module(GLM REQUIRED glm)

include_directories(
        ${GLEW_INCLUDE_DIRS}
        ${GLFW_INCLUDE_DIRS}
#        ${GLM_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

set(SOURCE_FILES
        main.c
        cglm_helper.c cglm_helper.h)

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

add_executable(MyCraft_Demo ${SOURCE_FILES} ${APPRES_OBJS})

target_link_libraries(MyCraft_Demo
        ${GLEW_LIBRARIES}
        ${GLFW_LIBRARIES}
#        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcglm.dll.a
#        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcglm.a
        )

if (WIN32 AND MINGW)
    # -lGL
    target_link_libraries(MyCraft_Demo opengl32)
endif()

message(STATUS "C Compiler: " ${CMAKE_C_COMPILER})
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "External Object: ${APPRES_OBJS}")
message(STATUS "External library: " ${GLEW_LIBRARIES} " " ${GLFW_LIBRARIES})
