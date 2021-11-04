# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Cata)

if ("${ASCEMU_NUMBER}" GREATER_EQUAL 3)
set(SRC_CATA_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_Lost_City_Of_Tolvir.cpp
    ${PATH_PREFIX}/Instance_Blackrock_Caverns.cpp
    ${PATH_PREFIX}/Instance_Grim_Batol.cpp
    ${PATH_PREFIX}/Instance_Halls_Of_Origination.cpp
    ${PATH_PREFIX}/Instance_Throne_Of_Tides.cpp
    ${PATH_PREFIX}/Instance_The_Stonecore.cpp
    ${PATH_PREFIX}/Instance_Vortex_Pinnacle.cpp
    ${PATH_PREFIX}/Instance_Zul_Aman_Cata.cpp
    ${PATH_PREFIX}/Instance_Zul_Gurub_Cata.cpp
    ${PATH_PREFIX}/Instance_Well_Of_Eternity.cpp
    ${PATH_PREFIX}/Instance_Hour_Of_Twilight.cpp
    ${PATH_PREFIX}/Instance_End_Time.cpp
    ${PATH_PREFIX}/Raid_Baradin_Hold.cpp
    ${PATH_PREFIX}/Raid_Bastion_Of_Twilight.cpp
    ${PATH_PREFIX}/Raid_Blackwing_Descent.cpp
    ${PATH_PREFIX}/Raid_DragonSoul.cpp
    ${PATH_PREFIX}/Raid_Firelands.cpp
    ${PATH_PREFIX}/Instance_Lost_City_Of_Tolvir.h
    ${PATH_PREFIX}/Instance_Blackrock_Caverns.h
    ${PATH_PREFIX}/Instance_Grim_Batol.h
    ${PATH_PREFIX}/Instance_Halls_Of_Origination.h
    ${PATH_PREFIX}/Instance_Throne_Of_Tides.h
    ${PATH_PREFIX}/Instance_The_Stonecore.h
    ${PATH_PREFIX}/Instance_Vortex_Pinnacle.h
    ${PATH_PREFIX}/Instance_Zul_Aman_Cata.h
    ${PATH_PREFIX}/Instance_Zul_Gurub_Cata.h
    ${PATH_PREFIX}/Instance_Well_Of_Eternity.h
    ${PATH_PREFIX}/Instance_Hour_Of_Twilight.h
    ${PATH_PREFIX}/Instance_End_Time.h
    ${PATH_PREFIX}/Raid_Baradin_Hold.h
    ${PATH_PREFIX}/Raid_Bastion_Of_Twilight.h
    ${PATH_PREFIX}/Raid_Blackwing_Descent.h
    ${PATH_PREFIX}/Raid_DragonSoul.h
    ${PATH_PREFIX}/Raid_Firelands.h
)
endif ()

source_group(Cata FILES ${SRC_CATA_SCRIPTS_FILES})
unset(PATH_PREFIX)
