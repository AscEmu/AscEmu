# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Kalimdor/RazorfenDowns)

set(SRC_KALIMDOR_RAZORFENDOWNS_FILES
   ${PATH_PREFIX}/RazorfenDowns.cpp
   ${PATH_PREFIX}/RazorfenDowns.h
)

source_group(Kalimdor\\RazorfenDowns FILES ${SRC_KALIMDOR_RAZORFENDOWNS_FILES})
unset(PATH_PREFIX)
