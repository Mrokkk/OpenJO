#============================================================================
# Copyright (C) 2013 - 2018, OpenJK contributors
#
# This file is part of the OpenJK source code.
#
# OpenJK is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#============================================================================

cmake_minimum_required(VERSION 3.1)

project(OpenJO)

set(IN_OPENJO ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

#=============================================================================
#
# Customizable options
#
#=============================================================================
include(CMakeDependentOption)
option(BUILD_PORTABLE "Build portable version (does not read or write files from your user/home directory" OFF)
option(BUILD_TESTS "Whether to build automatic unit tests (requires Boost)" OFF)
option(DECODE_STACKTRACE "Add ability to print decoded stacktrace when crash is encountered" ON)
option(NONFATAL_ASSERT "Assertion failures do not crash the game" ON)
cmake_dependent_option(NONFATAL_ASSERT_STACKTRACE "Nonfatal assertion failures will print stacktrace" ON "DECODE_STACKTRACE" OFF)

# Configure the use of bundled libraries.  By default, we assume the user is on
# a platform that does not require any bundling.
#
# Note that we always use the bundled copy of minizip, since it is modified to
# use Z_Malloc.

set(USE_INTERNAL_OPENAL_DEFAULT OFF)
set(USE_INTERNAL_ZLIB_DEFAULT   OFF)
set(USE_INTERNAL_PNG_DEFAULT    OFF)
set(USE_INTERNAL_JPEG_DEFAULT   OFF)
set(USE_INTERNAL_SDL2_DEFAULT   OFF)

if(USE_INTERNAL_LIBS OR WIN32)
    set(USE_INTERNAL_OPENAL_DEFAULT ON)
    set(USE_INTERNAL_ZLIB_DEFAULT   ON)
    set(USE_INTERNAL_PNG_DEFAULT    ON)
    set(USE_INTERNAL_JPEG_DEFAULT   ON)
    set(USE_INTERNAL_SDL2_DEFAULT   ON)
endif()

if(APPLE)
    set(USE_INTERNAL_JPEG_DEFAULT ON)
    set(USE_INTERNAL_PNG_DEFAULT  ON)
endif()

option(USE_INTERNAL_OPENAL "If set, use bundled OpenAL."  ${USE_INTERNAL_OPENAL_DEFAULT})
option(USE_INTERNAL_ZLIB   "If set, use bundled zlib."    ${USE_INTERNAL_ZLIB_DEFAULT})
option(USE_INTERNAL_PNG    "If set, use bundled libpng."  ${USE_INTERNAL_PNG_DEFAULT})
option(USE_INTERNAL_JPEG   "If set, use bundled libjpeg." ${USE_INTERNAL_JPEG_DEFAULT})
option(USE_INTERNAL_SDL2   "If set, use bundled SDL2."    ${USE_INTERNAL_SDL2_DEFAULT})

#=============================================================================
#
# Custom CMake Modules needed
#
#=============================================================================
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake/Modules")

include(ccache)
include(CompileCommands)

#=============================================================================
#
# Architecture/OS defines
#
#=============================================================================
# ${ARCHITECTURE} must match ARCH_STRING in q_platform.h,
# and is used in DLL names (jagamex86.dll, jagamex86.dylib, jagamei386.so).
if(WIN32)
    set(X86 ON)
    if(CMAKE_SIZEOF_VOID_P MATCHES "8")
        set(ARCHITECTURE "x86_64")
        set(WIN64 TRUE)
    else()
        set(ARCHITECTURE "x86")
        set(WIN64 FALSE)
    endif()
else()
    set(X86 OFF)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
        set(ARCHITECTURE "arm64")
        add_definitions(-DPNG_ARM_NEON_OPT=0)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
        set(ARCHITECTURE "arm")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
        set(X86 ON)
        if(APPLE)
            set(ARCHITECTURE "x86")
        else()
            # e.g. Linux
            set(ARCHITECTURE "i386")
        endif()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86.64$")
        set(X86 ON)
        set(ARCHITECTURE "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc")
        set(ARCHITECTURE "ppc")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc64")
        set(ARCHITECTURE "ppc64")
    else()
        set(ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

message(STATUS "Architecture is ${ARCHITECTURE}")
set(ARCHITECTURE "${ARCHITECTURE}" CACHE STRING "Architecture")

if(WIN32 AND CMAKE_VERSION VERSION_LESS "3.4")
    message(WARNING "Building on Windows platform with CMake version less than 3.4 is deprecated. Manifest file will fail to be included.")
endif()

#=============================================================================
#
# Binary names
#
#=============================================================================
set(ENGINE_NAME "openjo_sp.${ARCHITECTURE}")
set(GAME_NAME "jospgame${ARCHITECTURE}")
set(RENDERER_VANILLA_NAME "rdjosp-vanilla_${ARCHITECTURE}")

#=============================================================================
#
# Paths
#
#=============================================================================
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(GSL_DIR "${LIB_DIR}/gsl-lite/include")

include(InstallConfig)

#=============================================================================
#
# Compiler definitions/flags
#
#=============================================================================

# Operating system settings
if(WIN64)
    set(SHARED_DEFINES ${SHARED_DEFINES} "WIN64")
endif()

if(APPLE)
    set(SHARED_DEFINES ${SHARED_DEFINES} "MACOS_X")
endif()

if(NOT WIN32 AND NOT APPLE)
    set(SHARED_DEFINES ${SHARED_DEFINES} "ARCH_STRING=\"${ARCHITECTURE}\"")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "BSD")
    add_definitions(-DIOAPI_NO_64)
endif()

# Compiler settings
if(MSVC)
    set(SHARED_DEFINES ${SHARED_DEFINES} "NOMINMAX")
    set(SHARED_DEFINES ${SHARED_DEFINES} "_CRT_SECURE_NO_WARNINGS")
    set(SHARED_DEFINES ${SHARED_DEFINES} "_SCL_SECURE_NO_WARNINGS")
    set(SHARED_DEFINES ${SHARED_DEFINES} "_CRT_NONSTDC_NO_DEPRECATE")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:SSE2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

    # We don't try to control symbol visibility under MSVC.
    set(OPENJK_VISIBILITY_FLAGS "")
elseif (("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang"))
    # I hope this doesn't come back to bite me in the butt later on.
    # Realistically though, can the C and CXX compilers be different?

    # Visibility can't be set project-wide -- it needs to be specified on a
    # per-target basis.  This is primarily due to the bundled copy of ZLib.
    # ZLib explicitly declares symbols hidden, rather than defaulting to hidden.
    #
    # Note that -fvisibility=hidden is stronger than -fvisibility-inlines-hidden.
    set(OPENJK_VISIBILITY_FLAGS "-fvisibility=hidden")

    # removes the -rdynamic flag at linking (which causes crashes for some reason)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

    if (X86)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
    endif()

    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

    set(CMAKE_C_FLAGS_DEBUG "-O2 -ggdb3")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -DNDEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

    set(CMAKE_CXX_FLAGS_DEBUG "-O2 -ggdb3")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    set(WARNING_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-comment")

    if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} -fsigned-char")
        if (X86)
            # "x86 vm will crash without -mstackrealign since MMX
            # instructions will be used no matter what and they
            # corrupt the frame pointer in VM calls"
            # -ioquake3 Makefile
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mstackrealign -mfpmath=sse")
        endif()

        if(WIN32)
            # Link libgcc and libstdc++ statically
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
        endif()
    elseif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} -fsigned-char")
    endif()

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -Wno-invalid-offsetof -Wno-write-strings -fsigned-char -Wno-class-memaccess")
        if (X86)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mstackrealign -mfpmath=sse")
        endif()
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -Wno-invalid-offsetof -Wno-write-strings -fsigned-char")
    endif()
else()
    message(ERROR "Unsupported compiler")
endif()

set(SHARED_DEFINES ${SHARED_DEFINES} "$<$<CONFIG:Debug>:_DEBUG>")
set(SHARED_DEFINES ${SHARED_DEFINES} "$<$<NOT:$<CONFIG:Debug>>:FINAL_BUILD>")

if(NONFATAL_ASSERT)
    set(SHARED_DEFINES ${SHARED_DEFINES} "OPENJO_NONFATAL_ASSERT")
endif()

if(NONFATAL_ASSERT_STACKTRACE)
    set(SHARED_DEFINES ${SHARED_DEFINES} "OPENJO_NONFATAL_ASSERT_STACKTRACE")
endif()

#=============================================================================
#
# Generate version file
#
#=============================================================================

# Reproducible builds
# https://reproducible-builds.org/specs/source-date-epoch/
if ("$ENV{SOURCE_DATE_EPOCH}" STREQUAL "")
    message(STATUS "SOURCE_DATE_EPOCH is not set: SOURCE_DATE will be set to the compile-time date")
else()
    execute_process(
        COMMAND "date" "--date=@$ENV{SOURCE_DATE_EPOCH}" "+%b %_d %Y"
        OUTPUT_VARIABLE SOURCE_DATE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "SOURCE_DATE_EPOCH is set ($ENV{SOURCE_DATE_EPOCH}): SOURCE_DATE set to \"${SOURCE_DATE}\"")
endif()

# Current Git SHA1 hash
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_HASH)
message(STATUS "Git revision is ${GIT_HASH}")

configure_file(${CMAKE_SOURCE_DIR}/src/qcommon/q_version.h.in ${CMAKE_BINARY_DIR}/src/qcommon/q_version.h @ONLY)

#=============================================================================
#
# Adding all of the projects
#
#=============================================================================
if(USE_INTERNAL_JPEG)
    add_subdirectory(lib/jpeg-9a)
else()
    find_package(JPEG REQUIRED)
endif()

if(USE_INTERNAL_ZLIB)
    add_subdirectory(lib/zlib)
else()
    find_package(ZLIB REQUIRED)
endif()

if(USE_INTERNAL_PNG)
    add_subdirectory(lib/libpng)
else()
    find_package(PNG REQUIRED)
endif()

# Always use bundled minizip (sets MINIZIP_{LIBRARIES,INCLUDE_DIR})
add_subdirectory(lib/minizip)

# Add projects
add_subdirectory(${SRC_DIR})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory("tests")
endif()
