# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

if(NOT BUILD_MAPSCRIPTS )
  message("\tMapScripts was skipped because BUILD_MAPSCRIPTS is unchecked.")
  return()
endif()

project(MapScripts CXX)

include(EasternKingdoms/AlteracValley/CMakeLists.txt)
include(EasternKingdoms/BlackrockMountain/BlackrockDepths/CMakeLists.txt)
include(EasternKingdoms/BlackrockMountain/BlackrockSpire/CMakeLists.txt)
include(EasternKingdoms/BlackrockMountain/BlackwingLair/CMakeLists.txt)
include(EasternKingdoms/BlackrockMountain/MoltenCore/CMakeLists.txt)
include(EasternKingdoms/Deadmines/CMakeLists.txt)
include(EasternKingdoms/Gnomregan/CMakeLists.txt)
include(EasternKingdoms/Karazhan/CMakeLists.txt)
include(EasternKingdoms/MagisterTerrace/CMakeLists.txt)
include(EasternKingdoms/ScarletEnclave/CMakeLists.txt)
include(EasternKingdoms/ScarletMonastery/CMakeLists.txt)
include(EasternKingdoms/Scholomance/CMakeLists.txt)
include(EasternKingdoms/ShadowfangKeep/CMakeLists.txt)
include(EasternKingdoms/Stratholme/CMakeLists.txt)
include(EasternKingdoms/SunkenTemple/CMakeLists.txt)
include(EasternKingdoms/SunwellPlateau/CMakeLists.txt)
include(EasternKingdoms/TheStockade/CMakeLists.txt)
include(EasternKingdoms/Uldaman/CMakeLists.txt)
include(EasternKingdoms/ZulAman/CMakeLists.txt)
include(EasternKingdoms/ZulGurub/CMakeLists.txt)

include(Kalimdor/BlackfathomDeeps/CMakeLists.txt)
include(Kalimdor/CavernsOfTime/BattleForMountHyjal/CMakeLists.txt)
include(Kalimdor/CavernsOfTime/CullingOfStratholm/CMakeLists.txt)
include(Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/CMakeLists.txt)
include(Kalimdor/CavernsOfTime/TheBlackMorass/CMakeLists.txt)
include(Kalimdor/DireMaul/CMakeLists.txt)
include(Kalimdor/Maraudon/CMakeLists.txt)
include(Kalimdor/OnyxiasLair/CMakeLists.txt)
include(Kalimdor/RagefireChasm/CMakeLists.txt)
include(Kalimdor/RazorfenDowns/CMakeLists.txt)
include(Kalimdor/RazorfenKraul/CMakeLists.txt)
include(Kalimdor/RuinsOfAhnQiraj/CMakeLists.txt)
include(Kalimdor/TempleOfAhnQiraj/CMakeLists.txt)
include(Kalimdor/WailingCaverns/CMakeLists.txt)
include(Kalimdor/ZulFarrak/CMakeLists.txt)

include(Northrend/IceCrownCitadel/CMakeLists.txt)

include(Outland/TempestKeep/TheArcatraz/CMakeLists.txt)
include(Outland/TempestKeep/TheBotanica/CMakeLists.txt)
include(Outland/TempestKeep/TheEye/CMakeLists.txt)
include(Outland/TempestKeep/TheMechanar/CMakeLists.txt)

include(Outland/Auchindoun/AuchenaiCrypts/CMakeLists.txt)
include(Outland/Auchindoun/ManaTombs/CMakeLists.txt)
include(Outland/Auchindoun/SethekkHalls/CMakeLists.txt)
include(Outland/Auchindoun/ShadowLabyrinth/CMakeLists.txt)

set(SRC_BASIC
   Setup.cpp
   Setup.h
   ../Common/Instance_Base.cpp 
   ../Common/Instance_Base.h
   ../Common/Base.cpp
   ../Common/Base.h
   ../Common/AITimerCollection.cpp
   ../Common/AITimerCollection.h
   ../../world/Server/LazyTimer.cpp
   ../../world/Server/ServerState.cpp
)

set(sources
   ${SRC_BASIC}
   
   ${SRC_EASTERKINGDOMS_ALTERACVALLEY_FILES}
   ${SRC_EASTERKINGDOMS_BM_BLACKROCKDEPTHS_FILES}
   ${SRC_EASTERKINGDOMS_BM_BLACKROCKSPIRE_FILES}
   ${SRC_EASTERKINGDOMS_BM_BLACKWINGLAIR_FILES}
   ${SRC_EASTERKINGDOMS_BM_MOLTENCORE_FILES}
   ${SRC_EASTERKINGDOMS_DEADMINES_FILES}
   ${SRC_EASTERKINGDOMS_GNOMREGAN_FILES}
   ${SRC_EASTERKINGDOMS_KARAZHAN_FILES}
   ${SRC_EASTERKINGDOMS_MAGISTERTERRACE_FILES}
   ${SRC_EASTERKINGDOMS_SCARLETENCLAVE_FILES}
   ${SRC_EASTERKINGDOMS_SCARLETMONASTERY_FILES}
   ${SRC_EASTERKINGDOMS_SCHOLOMANCE_FILES}
   ${SRC_EASTERKINGDOMS_SHADOWFANGKEEP_FILES}
   ${SRC_EASTERKINGDOMS_STRATHOLME_FILES}
   ${SRC_EASTERKINGDOMS_SUNKENTEMPLE_FILES}
   ${SRC_EASTERKINGDOMS_SUNWELLPLATEAU_FILES}
   ${SRC_EASTERKINGDOMS_THESTOCKADE_FILES}
   ${SRC_EASTERKINGDOMS_ULDAMAN_FILES}
   ${SRC_EASTERKINGDOMS_ZULAMAN_FILES}
   ${SRC_EASTERKINGDOMS_ZULGURUB_FILES}
   
   ${SRC_KALIMDOR_BLACKFATHOMDEEPS_FILES}
   ${SRC_KALIMDOR_COT_BATTLEFORMOUNTHYJAL_FILES}
   ${SRC_KALIMDOR_COT_CULLINGOFSTRATHOLM_FILES}
   ${SRC_KALIMDOR_COT_ESCAPEFROMDURNHOLDKEEP_FILES}
   ${SRC_KALIMDOR_COT_BLACKMORASS_FILES}
   ${SRC_KALIMDOR_DIREMAUL_FILES}
   ${SRC_KALIMDOR_MARAUDON_FILES}
   ${SRC_KALIMDOR_ONYXIASLIAR_FILES}
   ${SRC_KALIMDOR_RAGEFIRECHASM_FILES}
   ${SRC_KALIMDOR_RAZORFENDOWNS_FILES}
   ${SRC_KALIMDOR_RAZORFENKRAUL_FILES}
   ${SRC_KALIMDOR_RUINSOFAHNQIRAJ_FILES}
   ${SRC_KALIMDOR_TEMPLEOFAHNQIRAJ_FILES}
   ${SRC_KALIMDOR_WAILINGCAVERNS_FILES}
   ${SRC_KALIMDOR_ZULFARAK_FILES}
   
   ${SRC_NORTHREND_ICC_FILES}
   
   ${SRC_OUTLAND_TK_THEARCATRAZ_FILES}
   ${SRC_OUTLAND_TK_THEBOTANICA_FILES}
   ${SRC_OUTLAND_TK_THEEYE_FILES}
   ${SRC_OUTLAND_TK_THEMECHANAR_FILES}
   ${SRC_OUTLAND_AU_AUCHENAICRYPTS_FILES}
   ${SRC_OUTLAND_AU_MANATOMBS_FILES}
   ${SRC_OUTLAND_AU_SETHEKKHALLS_FILES}
   ${SRC_OUTLAND_AU_SHADOWLABYRINTH_FILES}
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

add_library(${PROJECT_NAME} SHARED ${sources})

target_link_libraries(${PROJECT_NAME} world shared)

if(USE_PCH AND PCHSupport_FOUND)
    ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})
unset(sources)
