# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

# In-house MPQ archive reader for every AscEmu version's tools, replacing
# StormLib (Cata/Mop) and the old libmpq archive parser (Classic/TBC/WotLK)
# alike. include/mpqlib/codecs and src/codecs hold from-scratch C++23 ports of
# AscEmu's original libmpq codec vendoring (PKWARE DCL implode, Huffman,
# ADPCM wave) - see those headers for how faithfully each preserves the
# original bit-level algorithm. vendor/lzma is the public-domain LZMA SDK
# decoder (decode-only, no encoder), used for the LZMA (0x12) sector codec
# that some Cata/Mop archives - notably the Data/Cache/patch-*.MPQ hotfix
# archives - rely on.

set(LIBRARY_NAME mpqlib)

file(GLOB_RECURSE MPQLIB_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

set(MPQLIB_VENDOR_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lzma/LzmaDec.c
)

if (UNIX AND NOT IS_64BIT)
    add_compile_options(-D_FILE_OFFSET_BITS=64)
endif ()

ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} STATIC ${MPQLIB_SRCS} ${MPQLIB_VENDOR_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${LIBRARY_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lzma
    ${BZIP2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARIES}
)

target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_23)

# Minimal manual smoke-test executable: mpqlib_test <base.MPQ> [patch.MPQ ...] -- <fileInArchive>
add_executable(mpqlib_test test/mpqlib_test.cpp)
target_link_libraries(mpqlib_test PRIVATE ${LIBRARY_NAME})
install(TARGETS mpqlib_test RUNTIME DESTINATION ${ASCEMU_TOOLS_PATH})

unset(MPQLIB_SRCS)
unset(MPQLIB_VENDOR_SRCS)
