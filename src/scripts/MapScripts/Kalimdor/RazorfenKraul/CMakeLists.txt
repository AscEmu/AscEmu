# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Kalimdor/RazorfenKraul)

set(SRC_KALIMDOR_RAZORFENKRAUL_FILES
   ${PATH_PREFIX}/RazorfenKraul.cpp
   ${PATH_PREFIX}/RazorfenKraul.h
)

source_group(Kalimdor\\RazorfenKraul FILES ${SRC_KALIMDOR_RAZORFENKRAUL_FILES})
unset(PATH_PREFIX)
