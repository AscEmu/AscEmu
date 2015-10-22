# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(pcre C)

set(prefix ${DEPENDENCY_ROOT}/${PROJECT_NAME}/source)

set(sources
   pcre_chartables.c
   pcre_compile.c
   pcre_config.c
   pcre_dfa_exec.c
   pcre_exec.c
   pcre_fullinfo.c
   pcre_get.c
   pcre_globals.c
   pcre_info.c
   pcre_maketables.c
   pcre_newline.c
   pcre_ord2utf8.c
   pcre_refcount.c
   pcre_study.c
   pcre_tables.c
   pcre_try_flipped.c
   pcre_ucp_searchfuncs.c
   pcre_valid_utf8.c
   pcre_version.c
   pcre_xclass.c
   )

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)

include_directories(${DEPENDENCY_ROOT}/pcre/include)
add_library(${PROJECT_NAME} STATIC ${SRCS})

set(PCRE_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "PCRE Libraries.")
set(PCRE_INCLUDE_DIRS ${prefix} CACHE INTERNAL "PCRE include dirs.")
