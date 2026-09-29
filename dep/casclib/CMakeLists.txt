cmake_minimum_required(VERSION 3.25)
project(CascLib)

set(HEADER_FILES
    src/CascCommon.h
    src/CascLib.h
    src/CascPort.h
    src/common/Array.h
    src/common/Common.h
    src/common/Csv.h
    src/common/Directory.h
    src/common/FileStream.h
    src/common/FileTree.h
    src/common/ListFile.h
    src/common/Map.h
    src/common/Mime.h
    src/common/Path.h
    src/common/RootHandler.h
    src/common/Sockets.h
    src/jenkins/lookup.h
)

set(SRC_FILES
    src/common/Common.cpp
    src/common/Directory.cpp
    src/common/Csv.cpp
    src/common/FileStream.cpp
    src/common/FileTree.cpp
    src/common/ListFile.cpp
    src/common/Mime.cpp
    src/common/RootHandler.cpp
    src/common/Sockets.cpp
    src/hashes/md5.cpp
    src/hashes/sha1.cpp
    src/jenkins/lookup3.c
    src/overwatch/apm.cpp
    src/overwatch/cmf.cpp
    src/overwatch/aes.cpp
    src/CascDecompress.cpp
    src/CascDecrypt.cpp
    src/CascDumpData.cpp
    src/CascFiles.cpp
    src/CascFindFile.cpp
    src/CascIndexFiles.cpp
    src/CascOpenFile.cpp
    src/CascOpenStorage.cpp
    src/CascReadFile.cpp
    src/CascRootFile_Diablo3.cpp
    src/CascRootFile_Install.cpp
    src/CascRootFile_MNDX.cpp
    src/CascRootFile_Text.cpp
    src/CascRootFile_TVFS.cpp
    src/CascRootFile_OW.cpp
    src/CascRootFile_WoW.cpp
)

if (NOT TARGET zlib)
    message(FATAL_ERROR "CascLib requires the AscEmu zlib target")
endif()

set(PUBLIC_COMPILE_DEFINITIONS
    CASCLIB_NO_AUTO_LINK_LIBRARY
    CASCLIB_NODEBUG
    CASC_USE_SYSTEM_ZLIB
)

set(LINK_LIBS
    zlib
)

option(CASC_UNICODE "Compile UNICODE version instead of ANSI one (Visual Studio only)" OFF)

if(WIN32)
    list(APPEND LINK_LIBS wininet)

    if(CASC_UNICODE)
        message(STATUS "Build UNICODE version")
        add_definitions(-DUNICODE -D_UNICODE)
        list(APPEND PUBLIC_COMPILE_DEFINITIONS CASCLIB_UNICODE)
    else()
        message(STATUS "Build ANSI version")
    endif()

    list(APPEND PUBLIC_COMPILE_DEFINITIONS CASCLIB_DETECT_UNICODE_MISMATCHES)
endif()

message(STATUS "Build static linked library")

add_library(casc_static STATIC
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_link_libraries(casc_static
    PRIVATE
        ${LINK_LIBS}
)

target_include_directories(casc_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(casc_static
    PUBLIC
        ${PUBLIC_COMPILE_DEFINITIONS}
)

set_target_properties(casc_static PROPERTIES
    OUTPUT_NAME casc
)

if(APPLE)
    set_target_properties(casc_static PROPERTIES
        FRAMEWORK false
        PUBLIC_HEADER "src/CascLib.h src/CascPort.h"
        LINK_FLAGS "-framework Carbon"
    )
endif()

if(UNIX)
    set_target_properties(casc_static PROPERTIES
        VERSION 1.0.0
        SOVERSION 1
    )
endif()