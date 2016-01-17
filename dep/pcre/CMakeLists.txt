# Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(pcre C)
set(sources
   source/pcre_chartables.c
   source/pcre_compile.c
   source/pcre_config.c
   source/pcre_dfa_exec.c
   source/pcre_exec.c
   source/pcre_fullinfo.c
   source/pcre_get.c
   source/pcre_globals.c
   source/pcre_info.c
   source/pcre_maketables.c
   source/pcre_newline.c
   source/pcre_ord2utf8.c
   source/pcre_refcount.c
   source/pcre_study.c
   source/pcre_tables.c
   source/pcre_try_flipped.c
   source/pcre_ucp_searchfuncs.c
   source/pcre_valid_utf8.c
   source/pcre_version.c
   source/pcre_xclass.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(${PROJECT_NAME} STATIC ${sources})

set(PCRE_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "PCRE Libraries.")
set(PCRE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "PCRE include dir.")

unset(sources)