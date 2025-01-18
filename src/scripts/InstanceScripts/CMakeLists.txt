# Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>

if (NOT BUILD_INSTANCESCRIPTS)
    message("\tInstanceScripts was skipped because BUILD_INSTANCESCRIPTS is unchecked.")
    return()
endif ()

# set up our project name.
set(LIBRARY_NAME InstanceScripts)

file(GLOB SRC_LOCAL_DIR *.cpp *.h *.hpp)

file(GLOB SRC_CLASSIC_SCRIPTS_FILES Classic/*.cpp Classic/*.h)
file(GLOB SRC_CLASSIC_BLACKFATHOM_SCRIPTS_FILES Classic/BlackfathomDeeps/*.cpp Classic/BlackfathomDeeps/*.h)
file(GLOB SRC_CLASSIC_BLACKROCK_DEPTHS_SCRIPTS_FILES Classic/BlackrockMountain/BlackrockDepths/*.cpp Classic/BlackrockMountain/BlackrockDepths/*.h)
file(GLOB SRC_CLASSIC_BLACKROCK_SPIRE_SCRIPTS_FILES Classic/BlackrockMountain/BlackrockSpire/*.cpp Classic/BlackrockMountain/BlackrockSpire/*.h)
file(GLOB SRC_CLASSIC_BLACKWING_LAIR_SCRIPTS_FILES Classic/BlackrockMountain/BlackwingLair/*.cpp Classic/BlackrockMountain/BlackwingLair/*.h)
file(GLOB SRC_CLASSIC_MOLTENCORE_SCRIPTS_FILES Classic/BlackrockMountain/MoltenCore/*.cpp Classic/BlackrockMountain/MoltenCore/*.h)
file(GLOB SRC_CLASSIC_DEADMINES_SCRIPTS_FILES Classic/Deadmines/*.cpp Classic/Deadmines/*.h)
file(GLOB SRC_CLASSIC_DIREMAUL_SCRIPTS_FILES Classic/DireMaul/*.cpp Classic/DireMaul/*.h)
file(GLOB SRC_CLASSIC_GNOME_SCRIPTS_FILES Classic/Gnomeregan/*.cpp Classic/Gnomeregan/*.h)
file(GLOB SRC_CLASSIC_MARAUDON_SCRIPTS_FILES Classic/Maraudon/*.cpp Classic/Maraudon/*.h)
file(GLOB SRC_CLASSIC_ONYXIA_SCRIPTS_FILES Classic/OnyxiasLair/*.cpp Classic/OnyxiasLair/*.h)
file(GLOB SRC_CLASSIC_RAGEFIRECHASM_SCRIPTS_FILES Classic/RagefireChasm/*.cpp Classic/RagefireChasm/*.h)
file(GLOB SRC_CLASSIC_RAZORFEN_SCRIPTS_FILES Classic/RazorfenDowns/*.cpp Classic/RazorfenDowns/*.h)
file(GLOB SRC_CLASSIC_RAZORFENKRAUL_SCRIPTS_FILES Classic/RazorfenKraul/*.cpp Classic/RazorfenKraul/*.h)
file(GLOB SRC_CLASSIC_SCARLETMONES_SCRIPTS_FILES Classic/ScarletMonestary/*.cpp Classic/ScarletMonestary/*.h)
file(GLOB SRC_CLASSIC_SCHOLO_SCRIPTS_FILES Classic/Scholomance/*.cpp Classic/Scholomance/*.h)
file(GLOB SRC_CLASSIC_SHADOWFANG_SCRIPTS_FILES Classic/ShadowfangKeep/*.cpp Classic/ShadowfangKeep/*.h)
file(GLOB SRC_CLASSIC_STRATHOLME_SCRIPTS_FILES Classic/Stratholme/*.cpp Classic/Stratholme/*.h)
file(GLOB SRC_CLASSIC_SUNKENTEMPLE_SCRIPTS_FILES Classic/SunkenTemple/*.cpp Classic/SunkenTemple/*.h)
file(GLOB SRC_CLASSIC_STOCKADE_SCRIPTS_FILES Classic/TheStockade/*.cpp Classic/TheStockade/*.h)
file(GLOB SRC_CLASSIC_ULDAMAN_SCRIPTS_FILES Classic/Uldaman/*.cpp Classic/Uldaman/*.h)
file(GLOB SRC_CLASSIC_WAILING_SCRIPTS_FILES Classic/WailingCaverns/*.cpp Classic/WailingCaverns/*.h)
file(GLOB SRC_CLASSIC_ZUL_FARAK_SCRIPTS_FILES Classic/ZulFarak/*.cpp Classic/ZulFarak/*.h)
file(GLOB SRC_CLASSIC_ZUL_GURUB_SCRIPTS_FILES Classic/ZulGurub/*.cpp Classic/ZulGurub/*.h)

if ("${ASCEMU_NUMBER}" GREATER_EQUAL 1) # Tbc
file(GLOB SRC_TBC_SCRIPTS_FILES Tbc/*.cpp Tbc/*.h)
file(GLOB SRC_TBC_AUCHENAICRYPTS_SCRIPTS_FILES Tbc/Auchindoun/AuchenauCrypts/*.cpp Tbc/Auchindoun/AuchenauCrypts/*.h)
file(GLOB SRC_TBC_MANATOMBS_SCRIPTS_FILES Tbc/Auchindoun/ManaTombs/*.cpp Tbc/Auchindoun/ManaTombs/*.h)
file(GLOB SRC_TBC_SETHEKKHALLS_SCRIPTS_FILES Tbc/Auchindoun/SethekkHalls/*.cpp Tbc/Auchindoun/SethekkHalls/*.h)
file(GLOB SRC_TBC_SHADOWLABYRINTH_SCRIPTS_FILES Tbc/Auchindoun/ShadowLabyrinth/*.cpp Tbc/Auchindoun/ShadowLabyrinth/*.h)
file(GLOB SRC_TBC_BLACKTEMPLE_SCRIPTS_FILES Tbc/BlackTemple/*.cpp Tbc/BlackTemple/*.h)
file(GLOB SRC_TBC_COTBOFHYJAL_SCRIPTS_FILES Tbc/CavensOfTime/BattleOfMountHyjal/*.cpp Tbc/CavensOfTime/BattleOfMountHyjal/*.h)
file(GLOB SRC_TBC_COTBLACKMORASS_SCRIPTS_FILES Tbc/CavensOfTime/BlackMorass/*.cpp Tbc/CavensOfTime/BlackMorass/*.h)
file(GLOB SRC_TBC_COTOLDHILLS_SCRIPTS_FILES Tbc/CavensOfTime/OldHillsbradFoothills/*.cpp Tbc/CavensOfTime/OldHillsbradFoothills/*.h)
file(GLOB SRC_TBC_COILFANGSERPENTSHRINE_SCRIPTS_FILES Tbc/CoilfangReservoir/SerpentShrine/*.cpp Tbc/CoilfangReservoir/SerpentShrine/*.h)
file(GLOB SRC_TBC_COILFANGSLAVEPENS_SCRIPTS_FILES Tbc/CoilfangReservoir/SlavePens/*.cpp Tbc/CoilfangReservoir/SlavePens/*.h)
file(GLOB SRC_TBC_COILFANGSTEAMVAULT_SCRIPTS_FILES Tbc/CoilfangReservoir/SteamVault/*.cpp Tbc/CoilfangReservoir/SteamVault/*.h)
file(GLOB SRC_TBC_COILFANGUNDERBOG_SCRIPTS_FILES Tbc/CoilfangReservoir/Underbog/*.cpp Tbc/CoilfangReservoir/Underbog/*.h)
file(GLOB SRC_TBC_GRUULSLAIR_SCRIPTS_FILES Tbc/GruulsLair/*.cpp Tbc/GruulsLair/*.h)
file(GLOB SRC_TBC_BLOODFURNACE_SCRIPTS_FILES Tbc/HellfireCitadel/BloodFurnace/*.cpp Tbc/HellfireCitadel/BloodFurnace/*.h)
file(GLOB SRC_TBC_HELLFIRERAMPARTS_SCRIPTS_FILES Tbc/HellfireCitadel/HellfireRamparts/*.cpp Tbc/HellfireCitadel/HellfireRamparts/*.h Tbc/HellfireCitadel/HellfireRamparts/*.hpp)
file(GLOB SRC_TBC_MAGTHERIDONSLAIR_SCRIPTS_FILES Tbc/HellfireCitadel/MagtheridonsLair/*.cpp Tbc/HellfireCitadel/MagtheridonsLair/*.h Tbc/HellfireCitadel/MagtheridonsLair/*.hpp)
file(GLOB SRC_TBC_SHATTREDHALLS_SCRIPTS_FILES Tbc/HellfireCitadel/ShatteredHalls/*.cpp Tbc/HellfireCitadel/ShatteredHalls/*.h)
file(GLOB SRC_TBC_KARAZHAN_SCRIPTS_FILES Tbc/Karazhan/*.cpp Tbc/Karazhan/*.h)
file(GLOB SRC_TBC_MAGISTERSTERRACE_SCRIPTS_FILES Tbc/Sunwell/MagistersTerrace/*.cpp Tbc/Sunwell/MagistersTerrace/*.h)
file(GLOB SRC_TBC_SUNWELLPLATEAU_SCRIPTS_FILES Tbc/Sunwell/SunwellPlateau/*.cpp Tbc/Sunwell/SunwellPlateau/*.h)
file(GLOB SRC_TBC_ARCATRAZ_SCRIPTS_FILES Tbc/TempestKeep/Arcatraz/*.cpp Tbc/TempestKeep/Arcatraz/*.h)
file(GLOB SRC_TBC_BOTANICA_SCRIPTS_FILES Tbc/TempestKeep/Botanica/*.cpp Tbc/TempestKeep/Botanica/*.h)
file(GLOB SRC_TBC_MECHANAR_SCRIPTS_FILES Tbc/TempestKeep/Mechanar/*.cpp Tbc/TempestKeep/Mechanar/*.h)
file(GLOB SRC_TBC_THEEYE_SCRIPTS_FILES Tbc/TempestKeep/TheEye/*.cpp Tbc/TempestKeep/TheEye/*.h)
file(GLOB SRC_TBC_ZULAMAN_SCRIPTS_FILES Tbc/ZulAman/*.cpp Tbc/ZulAman/*.h)
endif ()

if ("${ASCEMU_NUMBER}" GREATER_EQUAL 2) # Wotlk
file(GLOB SRC_WOTLK_SCRIPTS_FILES Wotlk/*.cpp Wotlk/*.h Wotlk/*.hpp)
file(GLOB SRC_WOTLK_AHNKAHETOLDK_SCRIPTS_FILES Wotlk/AzjolNerub/Ahnkahet/*.cpp Wotlk/AzjolNerub/Ahnkahet/*.h)
file(GLOB SRC_WOTLK_AZJOLNERUB_SCRIPTS_FILES Wotlk/AzjolNerub/AzjolNerub/*.cpp Wotlk/AzjolNerub/AzjolNerub/*.h)
file(GLOB SRC_WOTLK_COTCULLINGOFSTRAT_SCRIPTS_FILES Wotlk/CavernsOfTime/*.cpp Wotlk/CavernsOfTime/*.h)
file(GLOB SRC_WOTLK_OBSIDUANSANCTUM_SCRIPTS_FILES Wotlk/ChamberOfAspects/ObsidianSanctum/*.cpp Wotlk/ChamberOfAspects/ObsidianSanctum/*.h)
file(GLOB SRC_WOTLK_RUBYSANCTUM_SCRIPTS_FILES Wotlk/ChamberOfAspects/RubySanctum/*.cpp Wotlk/ChamberOfAspects/RubySanctum/*.h)
file(GLOB SRC_WOTLK_TRIALOFCHAMPION_SCRIPTS_FILES Wotlk/CrusadersColiseum/TrialOfTheChampion/*.cpp Wotlk/CrusadersColiseum/TrialOfTheChampion/*.h)
file(GLOB SRC_WOTLK_TRIALOFCRUSADER_SCRIPTS_FILES Wotlk/CrusadersColiseum/TrialOfTheCrusader/*.cpp Wotlk/CrusadersColiseum/TrialOfTheCrusader/*.h Wotlk/CrusadersColiseum/TrialOfTheCrusader/*.hpp)
file(GLOB SRC_WOTLK_DRAKTHARON_SCRIPTS_FILES Wotlk/DraktharonKeep/*.cpp Wotlk/DraktharonKeep/*.h)
file(GLOB SRC_WOTLK_FORGEOFSOULS_SCRIPTS_FILES Wotlk/FrozenHalls/ForgeOfSouls/*.cpp Wotlk/FrozenHalls/ForgeOfSouls/*.h)
file(GLOB SRC_WOTLK_HALLSOFREFLECTION_SCRIPTS_FILES Wotlk/FrozenHalls/HallsOfReflection/*.cpp Wotlk/FrozenHalls/HallsOfReflection/*.h)
file(GLOB SRC_WOTLK_PITOFSARON_SCRIPTS_FILES Wotlk/FrozenHalls/PitOfSaron/*.cpp Wotlk/FrozenHalls/PitOfSaron/*.h)
file(GLOB SRC_WOTLK_GUNDRAK_SCRIPTS_FILES Wotlk/Gundrak/*.cpp Wotlk/Gundrak/*.h)
file(GLOB SRC_WOTLK_ICECROWNCITADEL_SCRIPTS_FILES Wotlk/IcecrownCitadel/*.cpp Wotlk/IcecrownCitadel/*.h Wotlk/IcecrownCitadel/*.hpp)
file(GLOB SRC_WOTLK_NAXXRAMAS_SCRIPTS_FILES Wotlk/Naxxramas/*.cpp Wotlk/Naxxramas/*.h)
file(GLOB SRC_WOTLK_EYEOFETERNITY_SCRIPTS_FILES Wotlk/Nexus/EyeOfEternity/*.cpp Wotlk/Nexus/EyeOfEternity/*.h)
file(GLOB SRC_WOTLK_NEXUS_SCRIPTS_FILES Wotlk/Nexus/Nexus/*.cpp Wotlk/Nexus/Nexus/*.h)
file(GLOB SRC_WOTLK_OCULUS_SCRIPTS_FILES Wotlk/Nexus/Oculus/*.cpp Wotlk/Nexus/Oculus/*.h)
file(GLOB SRC_WOTLK_HALLSOFLIGHTNING_SCRIPTS_FILES Wotlk/Ulduar/HallsOfLightning/*.cpp Wotlk/Ulduar/HallsOfLightning/*.h)
file(GLOB SRC_WOTLK_HALLSOFSTONE_SCRIPTS_FILES Wotlk/Ulduar/HallsOfStone/*.cpp Wotlk/Ulduar/HallsOfStone/*.h)
file(GLOB SRC_WOTLK_ULDUAR_SCRIPTS_FILES Wotlk/Ulduar/Ulduar/*.cpp Wotlk/Ulduar/Ulduar/*.h)
file(GLOB SRC_WOTLK_UTGARDEKEEP_SCRIPTS_FILES Wotlk/UtgardeKeep/UtgardeKeep/*.cpp Wotlk/UtgardeKeep/UtgardeKeep/*.h)
file(GLOB SRC_WOTLK_UTGARDEPINACLE_SCRIPTS_FILES Wotlk/UtgardeKeep/UtgradePinnacle/*.cpp Wotlk/UtgardeKeep/UtgradePinnacle/*.h)
file(GLOB SRC_WOTLK_VAULTOFARCHAVON_SCRIPTS_FILES Wotlk/VaultOfArchavon/*.cpp Wotlk/VaultOfArchavon/*.h)
file(GLOB SRC_WOTLK_VIOLETHOLD_SCRIPTS_FILES Wotlk/VioletHold/*.cpp Wotlk/VioletHold/*.h Wotlk/VioletHold/*.hpp)
endif ()

if ("${ASCEMU_NUMBER}" GREATER_EQUAL 3) # Cata
file(GLOB SRC_CATA_SCRIPTS_FILES Cata/*.cpp Cata/*.h Cata/*.hpp)
endif ()

# required for linker.
set(SRC_INSTANCESCRIPTS
    ../../world/Server/ServerState.cpp
    ../../shared/Utilities/Util.cpp
)

set(INSTANCESCRIPTS_SRCS
    ${SRC_INSTANCESCRIPTS}
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
    ${SRC_LOCAL_DIR}

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

# source group.
ascemu_source_group(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBRARY_NAME} SHARED ${INSTANCESCRIPTS_SRCS})

target_include_directories(${LIBRARY_NAME} PUBLIC 
    ${OPENSSL_INCLUDE_DIR}
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

# include library dependencies.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC
    world
)

# generate precompiled header.
if (AE_USE_PCH)
    message(STATUS "Generate PCH for ${LIBRARY_NAME}")
    target_precompile_headers(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pchInstanceScripts.hpp)
endif ()

install(TARGETS ${LIBRARY_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH})

unset(INSTANCESCRIPTS_SRCS)
