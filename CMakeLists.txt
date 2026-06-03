#
# CMakeLists.txt -- Linux build for Heretic2R.
#
# Produces the same module layout as the original MSVC solution, as Linux shared
# objects loaded at runtime via dlopen():
#   build/      Heretic2R (exe), quake2.so, H2Common.so, ref_gl1.so, snd_sdl3.so
#   build/base/ gamex86.so, Player.so, "Client Effects.so"
#

cmake_minimum_required(VERSION 3.16)
project(Heretic2R C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_STANDARD 17)            # game scripting subsystem (uses <unordered_map>, etc.)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SRC      ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE  ${CMAKE_SOURCE_DIR}/include)
set(LINUXDIR ${SRC}/linux)

# --- SDL3 (built separately; see notes in CLAUDE.md / build instructions) ------
set(SDL3_ROOT "/tmp/sdl3-install" CACHE PATH "Prefix containing libSDL3.so")
find_library(SDL3_LIB NAMES SDL3 PATHS ${SDL3_ROOT}/lib ${SDL3_ROOT}/lib64 NO_DEFAULT_PATH)
if(NOT SDL3_LIB)
    find_library(SDL3_LIB NAMES SDL3)
endif()
if(NOT SDL3_LIB)
    message(FATAL_ERROR "libSDL3 not found. Build SDL 3.4.2 and set -DSDL3_ROOT=<prefix>.")
endif()
message(STATUS "Using SDL3: ${SDL3_LIB}")

find_package(OpenGL REQUIRED)

# --- Output layout -------------------------------------------------------------
set(BUILD_DIR  ${CMAKE_SOURCE_DIR}/build)
set(BASE_DIR   ${BUILD_DIR}/base)

# --- Common compile settings, applied to every target -------------------------
# Force-include the Win32/MSVC compatibility shim before every TU, and put the
# stub <windows.h> et al. first on the include path so the ~6 cross-platform
# files that include <windows.h> resolve to the shim.
add_compile_options(
    -include ${LINUXDIR}/linux_compat.h
    -fcommon            # old-style tentative globals shared across TUs
    -fms-extensions     # forward-declared enums; lax prototype tag scoping
    -fno-strict-aliasing
    -w                  # silence the (very large) legacy warning set
)
# GCC 14+ promotes these legacy-C constructs to hard errors; the code is
# clean under MSVC, so demote them back to (silenced) warnings. C-only.
add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=incompatible-pointer-types>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=int-conversion>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=implicit-function-declaration>
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=implicit-int>
    # C++ analog: the field-offset macros (FOFS/STOFS/...) cast null-based member
    # pointers to int. The offsets fit in int (32-bit-origin structs); -fpermissive
    # demotes the C++ narrowing error to a (silenced) warning.
    $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
)
add_compile_definitions(NDEBUG)

include_directories(BEFORE
    ${LINUXDIR}/compat   # stub windows.h / direct.h / io.h
    ${LINUXDIR}
)
include_directories(
    ${INCLUDE}
    ${INCLUDE}/glad-GL1.3
    ${SRC}/qcommon
    ${SRC}/game/src
    ${SRC}/Player/src
)

# Helper: strip the "lib" prefix so module names match the originals.
function(h2_module target outdir)
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${outdir}
        C_VISIBILITY_PRESET default)
endfunction()

# ============================== H2Common.so ===================================
file(GLOB H2COMMON_SRC ${SRC}/H2Common/src/*.c)
# q_Shared.c (BoxOnPlaneSide and other shared helpers) is compiled into several
# modules by the original MSVC solution; mirror that here.
list(APPEND H2COMMON_SRC ${SRC}/game/src/q_Shared.c)
add_library(H2Common SHARED ${H2COMMON_SRC})
target_include_directories(H2Common PRIVATE ${SRC}/H2Common/src)
h2_module(H2Common ${BUILD_DIR})
target_link_libraries(H2Common PRIVATE m)

# ============================== quake2.so (engine) ============================
file(GLOB_RECURSE QUAKE2_SRC ${SRC}/quake2/src/*.c)
# Drop the Windows-only platform files (replaced by src/linux equivalents).
list(FILTER QUAKE2_SRC EXCLUDE REGEX "/win32/(net_wins|q_shwin|sys_win)\\.c$")
list(APPEND QUAKE2_SRC
    ${LINUXDIR}/sys_unix.c
    ${LINUXDIR}/q_shunix.c
    ${LINUXDIR}/net_udp.c
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
target_link_libraries(quake2 PRIVATE H2Common ${SDL3_LIB} ${CMAKE_DL_LIBS} m pthread)

# ============================== ref_gl1.so (renderer) =========================
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
target_link_libraries(ref_gl1 PRIVATE H2Common ${SDL3_LIB} OpenGL::GL ${CMAKE_DL_LIBS} m)

# ============================== snd_sdl3.so (sound) ===========================
file(GLOB_RECURSE SND_SRC ${SRC}/snd_sdl3/src/*.c)
add_library(snd_sdl3 SHARED ${SND_SRC})
target_include_directories(snd_sdl3 PRIVATE ${SRC}/snd_sdl3/src ${SRC}/quake2/src/client)
h2_module(snd_sdl3 ${BUILD_DIR})
target_link_libraries(snd_sdl3 PRIVATE H2Common ${SDL3_LIB} m)

# ============================== gamex86.so ====================================
file(GLOB_RECURSE GAME_SRC ${SRC}/game/src/*.c)
# The scripting subsystem (game/src/Scripting/**) is C++.
file(GLOB_RECURSE GAME_CXX_SRC ${SRC}/game/src/*.cpp)
list(APPEND GAME_SRC ${GAME_CXX_SRC}
    ${SRC}/qcommon/Message.c
    ${SRC}/qcommon/p_dll.c
    ${SRC}/qcommon/Skeletons.c)
add_library(gamex86 SHARED ${GAME_SRC})
target_compile_definitions(gamex86 PRIVATE GAME_DLL)
target_include_directories(gamex86 PRIVATE ${SRC}/game/src)
h2_module(gamex86 ${BASE_DIR})
target_link_libraries(gamex86 PRIVATE H2Common quake2 m)

# ============================== Player.so =====================================
file(GLOB_RECURSE PLAYER_SRC ${SRC}/Player/src/*.c)
list(APPEND PLAYER_SRC ${SRC}/game/src/q_Shared.c)
add_library(Player SHARED ${PLAYER_SRC})
target_compile_definitions(Player PRIVATE PLAYER_DLL)
target_include_directories(Player PRIVATE ${SRC}/Player/src)
h2_module(Player ${BASE_DIR})
target_link_libraries(Player PRIVATE H2Common quake2 m)

# ============================== Client Effects.so =============================
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
    LIBRARY_OUTPUT_DIRECTORY ${BASE_DIR})
target_link_libraries(ClientEffects PRIVATE H2Common quake2 m)

# ============================== Heretic2R (launcher exe) ======================
add_executable(Heretic2R ${LINUXDIR}/launcher_main.c)
target_include_directories(Heretic2R PRIVATE ${SRC}/quake2/src)
set_target_properties(Heretic2R PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})
target_link_libraries(Heretic2R PRIVATE quake2 ${CMAKE_DL_LIBS})
