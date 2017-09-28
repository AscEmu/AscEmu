# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Kalimdor/RazorfenKraul)

set(SRC_KALIMDOR_RAZORFENKRAUL_FILES
   ${PATH_PREFIX}/RazorfenKraul.cpp
   ${PATH_PREFIX}/RazorfenKraul.h
)

source_group(Kalimdor\\RazorfenKraul FILES ${SRC_KALIMDOR_RAZORFENKRAUL_FILES})
unset(PATH_PREFIX)
