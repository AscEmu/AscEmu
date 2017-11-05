# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_INSTANCESCRIPTS )
  message("\tInstanceScripts was skipped because BUILD_INSTANCESCRIPTS is unchecked.")
  return()
endif()

# set up our project name
project(InstanceScripts CXX)

#sources -contains source files for our library.
set(sources
    Instance_Arcatraz.cpp 
    Instance_AuchenaiCrypts.cpp 
    Instance_AzjolNerub.cpp 
    Instance_BlackfathomDeeps.cpp 
    Instance_BlackMorass.cpp 
    Instance_BlackrockDepths.cpp 
    Instance_BlackrockSpire.cpp 
    Instance_BloodFurnace.cpp 
    Instance_Botanica.cpp 
    Instance_CullingOfStratholme.cpp 
    Instance_Deadmines.cpp 
    Instance_DireMaul.cpp 
    Instance_DrakTharonKeep.cpp 
    Instance_ForgeOfSouls.cpp 
    Instance_Gundrak.cpp 
    Instance_HallsOfLightning.cpp 
    Instance_HallsOfReflection.cpp 
    Instance_HallsOfStone.cpp 
    Instance_HellfireRamparts.cpp 
    Instance_MagistersTerrace.cpp 
    Instance_ManaTombs.cpp 
    Instance_Maraudon.cpp 
    Instance_Nexus.cpp 
    Instance_OldHillsbradFoothills.cpp 
    Instance_PitOfSaron.cpp
    Instance_RagefireChasm.cpp 
    Instance_RazorfenDowns.cpp 
    Instance_ScarletMonastery.cpp 
    Instance_Scholomance.cpp 
    Instance_SethekkHalls.cpp 
    Instance_ShadowLabyrinth.cpp 
    Instance_ShadowfangKeep.cpp 
    Instance_TheMechanar.cpp 
    Instance_TheShatteredHalls.cpp 
    Instance_TheSlavePens.cpp 
    Instance_TheSteamvault.cpp 
    Instance_TheUnderbog.cpp 
    Instance_TheVioletHold.cpp 
    Instance_Uldaman.cpp 
    Instance_UtgardeKeep.cpp
    Instance_UtgardePinnacle.cpp	
    Instance_WailingCaverns.cpp 
    Instance_TheStockade.cpp 
    Instance_ZulFarrak.cpp 
    Raid_BlackTemple.cpp 
    Raid_BlackwingLair.cpp 
    Raid_CoT_BattleOfMountHyjal.cpp 
    Raid_GruulsLair.cpp
    Raid_IceCrownCitadel.cpp
    Raid_Karazhan.cpp 
    Raid_Magtheridons_Lair.cpp 
    Raid_MoltenCore.cpp 
    Raid_Naxxramas.cpp 
    Raid_OnyxiasLair.cpp 
    Raid_SerpentshrineCavern.cpp
    Raid_SunwellPlateau.cpp  
    Raid_TheEye.cpp 
    Raid_TheObsidianSanctum.cpp 
    Raid_Ulduar.cpp 
    Raid_ZulGurub.cpp 
    Raid_WorldBosses.cpp 
    Raid_ZulAman.cpp 
    ../Common/Base.cpp
    ../Common/AITimerCollection.cpp
    Setup.cpp
    # Required for linker
    ../../world/Server/LazyTimer.cpp
    ../../world/Server/ServerState.cpp
    )

set(headers
    Instance_Arcatraz.h 
    Instance_AuchenaiCrypts.h 
    Instance_AzjolNerub.h 
    Instance_BlackfathomDeeps.h 
    Instance_BlackMorass.h 
    Instance_BlackrockDepths.h 
    Instance_BlackrockSpire.h 
    Instance_BloodFurnace.h 
    Instance_Botanica.h 
    Instance_CullingOfStratholme.h 
    Instance_Deadmines.h 
    Instance_DireMaul.h 
    Instance_DrakTharonKeep.h 
    Instance_ForgeOfSouls.h
    Instance_Gundrak.h 
    Instance_HallsOfLightning.h 
    Instance_HallsOfReflection.h 
    Instance_HallsOfStone.h 
    Instance_HellfireRamparts.h 
    Instance_MagistersTerrace.h 
    Instance_ManaTombs.h 
    Instance_Maraudon.h 
    Instance_Nexus.h 
    Instance_OldHillsbradFoothills.h 
    Instance_PitOfSaron.h
    Instance_RagefireChasm.h 
    Instance_RazorfenDowns.h 
    Instance_ScarletMonastery.h 
    Instance_Scholomance.h 
    Instance_SethekkHalls.h 
    Instance_ShadowLabyrinth.h 
    Instance_ShadowfangKeep.h 
    Instance_TheMechanar.h 
    Instance_TheShatteredHalls.h 
    Instance_TheSlavePens.h 
    Instance_TheSteamvault.h 
    Instance_TheUnderbog.h 
    Instance_TheVioletHold.h 
    Instance_Uldaman.h 
    Instance_UtgardeKeep.h
    Instance_UtgardePinnacle.h	
    Instance_WailingCaverns.h 
    Instance_TheStockade.h 
    Instance_ZulFarrak.h
    Raid_BlackTemple.h 
    Raid_BlackwingLair.h 
    Raid_CoT_BattleOfMountHyjal.h 
    Raid_GruulsLair.h
    Raid_IceCrownCitadel.h
    Raid_Karazhan.h 
    Raid_Magtheridons_Lair.h 
    Raid_MoltenCore.h 
    Raid_Naxxramas.h 
    Raid_OnyxiasLair.h 
    Raid_SerpentshrineCavern.h
    Raid_SunwellPlateau.h
    Raid_TheEye.h
    Raid_TheObsidianSanctum.h
    Raid_Ulduar.h
    Raid_ZulGurub.h 
    Raid_WorldBosses.h 
    Raid_ZulAman.h
    Setup.h
    ../Common/Base.h
    ../Common/AITimerCollection.h
    
    #Linker
    ../../shared/Util.cpp
)

include_directories(
   ${OPENSSL_INCLUDE_DIR}
   ${PCRE_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
   ${CMAKE_SOURCE_DIR}/src/collision
   ${CMAKE_SOURCE_DIR}/src/collision/Management
   ${CMAKE_SOURCE_DIR}/src/collision/Maps
   ${CMAKE_SOURCE_DIR}/src/collision/Models
   ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
   ${CMAKE_SOURCE_DIR}/src/shared
   ${CMAKE_SOURCE_DIR}/src/world
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${ZLIB_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

target_link_libraries(${PROJECT_NAME} world)

if(USE_PCH AND PCHSupport_FOUND)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
unset(headers)
