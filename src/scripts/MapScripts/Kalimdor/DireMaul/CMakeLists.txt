# Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Kalimdor/DireMaul)

set(SRC_KALIMDOR_DIREMAUL_FILES
   ${PATH_PREFIX}/DireMaul.cpp
   ${PATH_PREFIX}/DireMaul.h
)

source_group(Kalimdor\\DireMaul FILES ${SRC_KALIMDOR_DIREMAUL_FILES})
unset(PATH_PREFIX)
