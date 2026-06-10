#
# CMakeLists.txt -- Heretic2R build (Linux & Windows/MinGW).
#

cmake_minimum_required(VERSION 3.16)
project(Heretic2R C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SRC      ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE  ${CMAKE_SOURCE_DIR}/include)
set(LINUXDIR ${SRC}/linux)

# --- SDL3 ----------------------------------------------------------------------
if(WIN32)
    set(SDL3_ROOT "/root/win32-install" CACHE PATH "Prefix containing SDL3.dll")
else()
    set(SDL3_ROOT "/tmp/sdl3-install" CACHE PATH "Prefix containing libSDL3.so")
endif()

find_library(SDL3_LIB NAMES SDL3 PATHS ${SDL3_ROOT}/lib ${SDL3_ROOT}/bin NO_DEFAULT_PATH)
if(NOT SDL3_LIB)
    find_library(SDL3_LIB NAMES SDL3)
endif()
if(NOT SDL3_LIB)
    message(FATAL_ERROR "libSDL3 not found.")
endif()
message(STATUS "Using SDL3: ${SDL3_LIB}")

if(NOT WIN32)
    find_package(OpenGL REQUIRED)
endif()

# --- Output layout -------------------------------------------------------------
if(WIN32)
    set(BUILD_DIR  ${CMAKE_SOURCE_DIR}/build-win32)
else()
    set(BUILD_DIR  ${CMAKE_SOURCE_DIR}/build)
endif()
set(BASE_DIR   ${BUILD_DIR}/base)

# --- Common compile settings --------------------------------------------------
if(WIN32)
    add_compile_options(-include ${SRC}/win32_cross_compat.h)
else()
    add_compile_options(-include ${LINUXDIR}/linux_compat.h)
endif()

add_compile_options(
    -fcommon
    -fms-extensions
    -fno-strict-aliasing
    -w
)

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=incompatible-pointer-types>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=int-conversion>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=implicit-function-declaration>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=implicit-int>
    $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
)
add_compile_definitions(NDEBUG)
if(NOT WIN32)
    add_compile_definitions(_GNU_SOURCE)
endif()

if(NOT WIN32)
    include_directories(BEFORE
        ${LINUXDIR}/compat
        ${LINUXDIR}
    )
endif()

include_directories(
    ${INCLUDE}
    ${INCLUDE}/glad-GL1.3
    ${SRC}/qcommon
    ${SRC}/game/src
    ${SRC}/Player/src
    ${CMAKE_SOURCE_DIR}/include/cross_compat
)

function(h2_module target outdir)
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${outdir}
        RUNTIME_OUTPUT_DIRECTORY ${outdir}
        C_VISIBILITY_PRESET default)
endfunction()

# ============================== H2Common ======================================
file(GLOB H2COMMON_SRC ${SRC}/H2Common/src/*.c)
list(APPEND H2COMMON_SRC ${SRC}/game/src/q_Shared.c)
add_library(H2Common SHARED ${H2COMMON_SRC})
target_compile_definitions(H2Common PRIVATE H2COMMON)
target_include_directories(H2Common PRIVATE ${SRC}/H2Common/src)
h2_module(H2Common ${BUILD_DIR})
if(NOT WIN32)
    target_link_libraries(H2Common PRIVATE m)
endif()

# ============================== quake2 (engine) ===============================
file(GLOB_RECURSE QUAKE2_SRC ${SRC}/quake2/src/*.c)
if(WIN32)
    list(FILTER QUAKE2_SRC EXCLUDE REGEX "/linux/.*")
else()
    list(FILTER QUAKE2_SRC EXCLUDE REGEX "/win32/(net_wins|q_shwin|sys_win)\\.c$")
    list(APPEND QUAKE2_SRC
        ${LINUXDIR}/sys_unix.c
        ${LINUXDIR}/q_shunix.c
        ${LINUXDIR}/net_udp.c
    )
endif()

list(APPEND QUAKE2_SRC
    ${SRC}/qcommon/anorms.c
    ${SRC}/qcommon/netmsg_read.c
    ${SRC}/qcommon/netmsg_write.c
    ${SRC}/qcommon/p_dll.c
    ${SRC}/qcommon/Reference.c
    ${SRC}/qcommon/Skeletons.c
    ${SRC}/qcommon/turbsin.c
    ${SRC}/game/src/q_Shared.c
    ${INCLUDE}/libsmacker/smacker.c
)
add_library(quake2 SHARED ${QUAKE2_SRC})
target_compile_definitions(quake2 PRIVATE QUAKE2_DLL)
target_include_directories(quake2 PRIVATE
    ${SRC}/quake2/src
    ${SRC}/quake2/src/client
    ${SRC}/quake2/src/server)
h2_module(quake2 ${BUILD_DIR})
if(WIN32)
    target_link_libraries(quake2 PRIVATE H2Common ${SDL3_LIB} ws2_32 winmm)
else()
    target_link_libraries(quake2 PRIVATE H2Common ${SDL3_LIB} ${CMAKE_DL_LIBS} m pthread)
endif()

# ============================== ref_gl1 (renderer) ============================
file(GLOB_RECURSE REFGL1_SRC ${SRC}/ref_gl1/src/*.c)
list(APPEND REFGL1_SRC
    ${INCLUDE}/glad-GL1.3/glad.c
    ${SRC}/qcommon/anorms.c
    ${SRC}/qcommon/Reference.c
    ${SRC}/qcommon/Skeletons.c
    ${SRC}/qcommon/turbsin.c
    ${SRC}/game/src/q_Shared.c)
add_library(ref_gl1 SHARED ${REFGL1_SRC})
target_include_directories(ref_gl1 PRIVATE ${SRC}/ref_gl1/src ${SRC}/quake2/src)
h2_module(ref_gl1 ${BUILD_DIR})
if(WIN32)
    target_link_libraries(ref_gl1 PRIVATE H2Common ${SDL3_LIB} opengl32)
else()
    target_link_libraries(ref_gl1 PRIVATE H2Common ${SDL3_LIB} OpenGL::GL ${CMAKE_DL_LIBS} m)
endif()

# ============================== snd_sdl3 (sound) ==============================
file(GLOB_RECURSE SND_SRC ${SRC}/snd_sdl3/src/*.c)
add_library(snd_sdl3 SHARED ${SND_SRC})
target_include_directories(snd_sdl3 PRIVATE ${SRC}/snd_sdl3/src ${SRC}/quake2/src/client)
h2_module(snd_sdl3 ${BUILD_DIR})
target_link_libraries(snd_sdl3 PRIVATE H2Common ${SDL3_LIB})
if(NOT WIN32)
    target_link_libraries(snd_sdl3 PRIVATE m)
endif()

# ============================== gamex86 =======================================
file(GLOB_RECURSE GAME_SRC ${SRC}/game/src/*.c)
file(GLOB_RECURSE GAME_CXX_SRC ${SRC}/game/src/*.cpp)
list(APPEND GAME_SRC ${GAME_CXX_SRC}
    ${SRC}/qcommon/Message.c
    ${SRC}/qcommon/p_dll.c
    ${SRC}/qcommon/Skeletons.c)
add_library(gamex86 SHARED ${GAME_SRC})
target_compile_definitions(gamex86 PRIVATE GAME_DLL)
target_include_directories(gamex86 PRIVATE ${SRC}/game/src)
h2_module(gamex86 ${BASE_DIR})
target_link_libraries(gamex86 PRIVATE H2Common quake2)
if(NOT WIN32)
    target_link_libraries(gamex86 PRIVATE m)
endif()

# ============================== Player ========================================
file(GLOB_RECURSE PLAYER_SRC ${SRC}/Player/src/*.c)
list(APPEND PLAYER_SRC ${SRC}/game/src/q_Shared.c)
add_library(Player SHARED ${PLAYER_SRC})
target_compile_definitions(Player PRIVATE PLAYER_DLL)
target_include_directories(Player PRIVATE ${SRC}/Player/src)
h2_module(Player ${BASE_DIR})
target_link_libraries(Player PRIVATE H2Common quake2)
if(NOT WIN32)
    target_link_libraries(Player PRIVATE m)
endif()

# ============================== Client Effects ================================
file(GLOB_RECURSE CLFX_SRC "${SRC}/client effects/src/*.c")
list(APPEND CLFX_SRC
    ${SRC}/qcommon/anorms.c
    ${SRC}/qcommon/Message.c
    ${SRC}/qcommon/netmsg_read.c
    ${SRC}/qcommon/turbsin.c)
add_library(ClientEffects SHARED ${CLFX_SRC})
target_include_directories(ClientEffects PRIVATE "${SRC}/client effects/src" ${SRC}/quake2/src)
set_target_properties(ClientEffects PROPERTIES
    PREFIX "" OUTPUT_NAME "Client Effects"
    LIBRARY_OUTPUT_DIRECTORY ${BASE_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}
    C_VISIBILITY_PRESET default)
target_link_libraries(ClientEffects PRIVATE H2Common quake2)
if(NOT WIN32)
    target_link_libraries(ClientEffects PRIVATE m)
endif()

# ============================== Launcher ======================================
if(WIN32)
    set(LAUNCHER_SRC ${SRC}/launcher/src/main.c)
else()
    set(LAUNCHER_SRC ${LINUXDIR}/launcher_main.c)
endif()
add_executable(Heretic2R ${LAUNCHER_SRC})
target_include_directories(Heretic2R PRIVATE ${SRC}/quake2/src)
h2_module(Heretic2R ${BUILD_DIR})
target_link_libraries(Heretic2R PRIVATE quake2)
if(WIN32)
    set_target_properties(Heretic2R PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
