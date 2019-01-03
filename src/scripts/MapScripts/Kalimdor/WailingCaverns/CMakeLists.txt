# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Kalimdor/WailingCaverns)

set(SRC_KALIMDOR_WAILINGCAVERNS_FILES
   ${PATH_PREFIX}/WailingCaverns.cpp
   ${PATH_PREFIX}/WailingCaverns.h
)

source_group(Kalimdor\\WailingCaverns FILES ${SRC_KALIMDOR_WAILINGCAVERNS_FILES})
unset(PATH_PREFIX)
