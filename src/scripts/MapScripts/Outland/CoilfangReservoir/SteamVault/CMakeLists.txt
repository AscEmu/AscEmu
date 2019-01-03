# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Outland/CoilfangReservoir/SteamVault)

set(SRC_OUTLAND_CR_STEAMVAULT_FILES
   ${PATH_PREFIX}/SteamVault.cpp
   ${PATH_PREFIX}/SteamVault.h
)

source_group(Outland\\CoilfangReservoir\\SteamVault FILES ${SRC_OUTLAND_CR_STEAMVAULT_FILES})
unset(PATH_PREFIX)
