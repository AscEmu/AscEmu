# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

if (NOT BUILD_INSTANCESCRIPTS )
    message("\tInstanceScripts was skipped because BUILD_INSTANCESCRIPTS is unchecked.")
    return()
endif ()

# set up our project name
project(InstanceScripts CXX)

include(Classic/CMakeLists.txt)
include(Classic/BlackfathomDeeps/CMakeLists.txt)
include(Classic/BlackrockMountain/BlackrockDepths/CMakeLists.txt)
include(Classic/BlackrockMountain/BlackrockSpire/CMakeLists.txt)
include(Classic/BlackrockMountain/BlackwingLair/CMakeLists.txt)
include(Classic/BlackrockMountain/MoltenCore/CMakeLists.txt)
include(Classic/Deadmines/CMakeLists.txt)
include(Classic/DireMaul/CMakeLists.txt)
include(Classic/Gnomeregan/CMakeLists.txt)
include(Classic/Maraudon/CMakeLists.txt)
include(Classic/OnyxiasLair/CMakeLists.txt)
include(Classic/RagefireChasm/CMakeLists.txt)
include(Classic/RazorfenDowns/CMakeLists.txt)
include(Classic/RazorfenKraul/CMakeLists.txt)
include(Classic/ScarletMonestary/CMakeLists.txt)
include(Classic/Scholomance/CMakeLists.txt)
include(Classic/ShadowfangKeep/CMakeLists.txt)
include(Classic/Stratholme/CMakeLists.txt)
include(Classic/SunkenTemple/CMakeLists.txt)
include(Classic/TheStockade/CMakeLists.txt)
include(Classic/Uldaman/CMakeLists.txt)
include(Classic/WailingCaverns/CMakeLists.txt)
include(Classic/ZulFarak/CMakeLists.txt)
include(Classic/ZulGurub/CMakeLists.txt)
include(Tbc/CMakeLists.txt)
include(Tbc/Auchindoun/AuchenauCrypts/CMakeLists.txt)
include(Tbc/Auchindoun/ManaTombs/CMakeLists.txt)
include(Tbc/Auchindoun/SethekkHalls/CMakeLists.txt)
include(Tbc/Auchindoun/ShadowLabyrinth/CMakeLists.txt)
include(Tbc/BlackTemple/CMakeLists.txt)
include(Tbc/CavensOfTime/BattleOfMountHyjal/CMakeLists.txt)
include(Tbc/CavensOfTime/BlackMorass/CMakeLists.txt)
include(Tbc/CavensOfTime/OldHillsbradFoothills/CMakeLists.txt)
include(Tbc/CoilfangReservoir/SerpentShrine/CMakeLists.txt)
include(Tbc/CoilfangReservoir/SlavePens/CMakeLists.txt)
include(Tbc/CoilfangReservoir/SteamVault/CMakeLists.txt)
include(Tbc/CoilfangReservoir/Underbog/CMakeLists.txt)
include(TBC/GruulsLair/CMakeLists.txt)
include(Tbc/HellfireCitadel/BloodFurnace/CMakeLists.txt)
include(Tbc/HellfireCitadel/HellfireRamparts/CMakeLists.txt)
include(Tbc/HellfireCitadel/MagtheridonsLair/CMakeLists.txt)
include(Tbc/HellfireCitadel/ShatteredHalls/CMakeLists.txt)
include(Tbc/Karazhan/CMakeLists.txt)
include(Tbc/Sunwell/MagistersTerrace/CMakeLists.txt)
include(Tbc/Sunwell/SunwellPlateau/CMakeLists.txt)
include(Tbc/TempestKeep/Arcatraz/CMakeLists.txt)
include(Tbc/TempestKeep/Botanica/CMakeLists.txt)
include(Tbc/TempestKeep/Mechanar/CMakeLists.txt)
include(Tbc/TempestKeep/TheEye/CMakeLists.txt)
include(Tbc/ZulAman/CMakeLists.txt)
include(Wotlk/CMakeLists.txt)
include(Wotlk/AzjolNerub/Ahnkahet/CMakeLists.txt)
include(Wotlk/AzjolNerub/AzjolNerub/CMakeLists.txt)
include(Wotlk/CavernsOfTime/CMakeLists.txt)
include(Wotlk/ChamberOfAspects/ObsidianSanctum/CMakeLists.txt)
include(Wotlk/ChamberOfAspects/RubySanctum/CMakeLists.txt)
include(Wotlk/CrusadersColiseum/TrialOfTheChampion/CMakeLists.txt)
include(Wotlk/CrusadersColiseum/TrialOfTheCrusader/CMakeLists.txt)
include(Wotlk/DraktharonKeep/CMakeLists.txt)
include(Wotlk/FrozenHalls/ForgeOfSouls/CMakeLists.txt)
include(Wotlk/FrozenHalls/HallsOfReflection/CMakeLists.txt)
include(Wotlk/FrozenHalls/PitOfSaron/CMakeLists.txt)
include(Wotlk/Gundrak/CMakeLists.txt)
include(Wotlk/IcecrownCitadel/CMakeLists.txt)
include(Wotlk/Naxxramas/CMakeLists.txt)
include(Wotlk/Nexus/EyeOfEternity/CMakeLists.txt)
include(Wotlk/Nexus/Nexus/CMakeLists.txt)
include(Wotlk/Nexus/Oculus/CMakeLists.txt)
include(Wotlk/Ulduar/HallsOfLightning/CMakeLists.txt)
include(Wotlk/Ulduar/HallsOfStone/CMakeLists.txt)
include(Wotlk/Ulduar/Ulduar/CMakeLists.txt)
include(Wotlk/UtgardeKeep/UtgardeKeep/CMakeLists.txt)
include(Wotlk/UtgardeKeep/UtgradePinnacle/CMakeLists.txt)
include(Wotlk/VaultOfArchavon/CMakeLists.txt)
include(Wotlk/VioletHold/CMakeLists.txt)
include(Cata/CMakeLists.txt)

set(SRC_BASIC
    Setup.cpp
    Setup.h
    # required for linker
    ../../world/Server/ServerState.cpp
    ../../shared/Util.cpp
)

#sources - contains source files for our library.
set(sources
    ${SRC_BASIC}
    ${SRC_CLASSIC_SCRIPTS_FILES}
	${SRC_CLASSIC_BLACKFATHOM_SCRIPTS_FILES}
	${SRC_CLASSIC_BLACKROCK_DEPTHS_SCRIPTS_FILES}
	${SRC_CLASSIC_BLACKROCK_SPIRE_SCRIPTS_FILES}
	${SRC_CLASSIC_BLACKWING_LAIR_SCRIPTS_FILES}
	${SRC_CLASSIC_MOLTENCORE_SCRIPTS_FILES}
	${SRC_CLASSIC_DEADMINES_SCRIPTS_FILES}
	${SRC_CLASSIC_DIREMAUL_SCRIPTS_FILES}
	${SRC_CLASSIC_GNOME_SCRIPTS_FILES}
	${SRC_CLASSIC_MARAUDON_SCRIPTS_FILES}
	${SRC_CLASSIC_ONYXIA_SCRIPTS_FILES}
	${SRC_CLASSIC_RAGEFIRECHASM_SCRIPTS_FILES}
	${SRC_CLASSIC_RAZORFEN_SCRIPTS_FILES}
	${SRC_CLASSIC_RAZORFENKRAUL_SCRIPTS_FILES}
	${SRC_CLASSIC_SCARLETMONES_SCRIPTS_FILES}
	${SRC_CLASSIC_SCHOLO_SCRIPTS_FILES}
	${SRC_CLASSIC_SHADOWFANG_SCRIPTS_FILES}
	${SRC_CLASSIC_STRATHOLME_SCRIPTS_FILES}
	${SRC_CLASSIC_SUNKENTEMPLE_SCRIPTS_FILES}
	${SRC_CLASSIC_STOCKADE_SCRIPTS_FILES}
	${SRC_CLASSIC_ULDAMAN_SCRIPTS_FILES}
	${SRC_CLASSIC_WAILING_SCRIPTS_FILES}
	${SRC_CLASSIC_ZUL_FARAK_SCRIPTS_FILES}
	${SRC_CLASSIC_ZUL_GURUB_SCRIPTS_FILES}
    ${SRC_TBC_SCRIPTS_FILES}
	${SRC_TBC_AUCHENAICRYPTS_SCRIPTS_FILES}
	${SRC_TBC_MANATOMBS_SCRIPTS_FILES}
	${SRC_TBC_SETHEKKHALLS_SCRIPTS_FILES}
	${SRC_TBC_SHADOWLABYRINTH_SCRIPTS_FILES}
	${SRC_TBC_BLACKTEMPLE_SCRIPTS_FILES}
	${SRC_TBC_COTBOFHYJAL_SCRIPTS_FILES}
	${SRC_TBC_COTBLACKMORASS_SCRIPTS_FILES}
	${SRC_TBC_COTOLDHILLS_SCRIPTS_FILES}
	${SRC_TBC_COILFANGSERPENTSHRINE_SCRIPTS_FILES}
	${SRC_TBC_COILFANGSLAVEPENS_SCRIPTS_FILES}
	${SRC_TBC_COILFANGSTEAMVAULT_SCRIPTS_FILES}
	${SRC_TBC_COILFANGUNDERBOG_SCRIPTS_FILES}
	${SRC_TBC_GRUULSLAIR_SCRIPTS_FILES}
	${SRC_TBC_BLOODFURNACE_SCRIPTS_FILES}
	${SRC_TBC_HELLFIRERAMPARTS_SCRIPTS_FILES}
	${SRC_TBC_MAGTHERIDONSLAIR_SCRIPTS_FILES}
	${SRC_TBC_SHATTREDHALLS_SCRIPTS_FILES}
	${SRC_TBC_KARAZHAN_SCRIPTS_FILES}
	${SRC_TBC_MAGISTERSTERRACE_SCRIPTS_FILES}
	${SRC_TBC_SUNWELLPLATEAU_SCRIPTS_FILES}
	${SRC_TBC_ARCATRAZ_SCRIPTS_FILES}
	${SRC_TBC_BOTANICA_SCRIPTS_FILES}
	${SRC_TBC_MECHANAR_SCRIPTS_FILES}
	${SRC_TBC_THEEYE_SCRIPTS_FILES}
	${SRC_TBC_ZULAMAN_SCRIPTS_FILES}
    ${SRC_WOTLK_SCRIPTS_FILES}	
	${SRC_WOTLK_AHNKAHETOLDK_SCRIPTS_FILES}
	${SRC_WOTLK_AZJOLNERUB_SCRIPTS_FILES}
	${SRC_WOTLK_COTCULLINGOFSTRAT_SCRIPTS_FILES}
	${SRC_WOTLK_OBSIDUANSANCTUM_SCRIPTS_FILES}
	${SRC_WOTLK_RUBYSANCTUM_SCRIPTS_FILES}
	${SRC_WOTLK_TRIALOFCHAMPION_SCRIPTS_FILES}
	${SRC_WOTLK_TRIALOFCRUSADER_SCRIPTS_FILES}
	${SRC_WOTLK_DRAKTHARON_SCRIPTS_FILES}
	${SRC_WOTLK_FORGEOFSOULS_SCRIPTS_FILES}
	${SRC_WOTLK_HALLSOFREFLECTION_SCRIPTS_FILES}
	${SRC_WOTLK_PITOFSARON_SCRIPTS_FILES}
	${SRC_WOTLK_GUNDRAK_SCRIPTS_FILES}
	${SRC_WOTLK_ICECROWNCITADEL_SCRIPTS_FILES}
	${SRC_WOTLK_NAXXRAMAS_SCRIPTS_FILES}
	${SRC_WOTLK_EYEOFETERNITY_SCRIPTS_FILES}
	${SRC_WOTLK_NEXUS_SCRIPTS_FILES}
	${SRC_WOTLK_OCULUS_SCRIPTS_FILES}
	
	
	${SRC_WOTLK_HALLSOFLIGHTNING_SCRIPTS_FILES}
	${SRC_WOTLK_HALLSOFSTONE_SCRIPTS_FILES}
	${SRC_WOTLK_ULDUAR_SCRIPTS_FILES}
	${SRC_WOTLK_UTGARDEKEEP_SCRIPTS_FILES}
	${SRC_WOTLK_UTGARDEPINACLE_SCRIPTS_FILES}
	${SRC_WOTLK_VAULTOFARCHAVON_SCRIPTS_FILES}
	${SRC_WOTLK_VIOLETHOLD_SCRIPTS_FILES}
	
	
    ${SRC_CATA_SCRIPTS_FILES}	
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
    ${CMAKE_SOURCE_DIR}/dep/utf8cpp
)

add_library(${PROJECT_NAME} SHARED ${sources} ${headers})

target_link_libraries(${PROJECT_NAME} world)

if (USE_PCH)
    ##gen_cxx_pch(${PROJECT_NAME} Setup.h ${CMAKE_CURRENT_SOURCE_DIR}/Setup.cpp)
endif ()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

if (USE_PCH)
    target_precompile_headers(${PROJECT_NAME}
    PRIVATE
    <Server/Script/CreatureAIScript.h>
    Setup.h
)
endif ()

unset(sources)
unset(headers)
