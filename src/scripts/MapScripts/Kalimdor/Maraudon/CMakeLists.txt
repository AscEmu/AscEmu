# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Kalimdor/Maraudon)

set(SRC_KALIMDOR_MARAUDON_FILES
   ${PATH_PREFIX}/Maraudon.cpp
   ${PATH_PREFIX}/Maraudon.h
)

source_group(Kalimdor\\Maraudon FILES ${SRC_KALIMDOR_MARAUDON_FILES})
unset(PATH_PREFIX)
