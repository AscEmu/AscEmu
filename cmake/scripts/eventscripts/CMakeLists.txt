if(NOT BUILD_EVENTSCRIPTS )
  message("\tEventScripts was skipped because BUILD_EVENTSCRIPTS is unchecked.")
  return()
endif(NOT BUILD_EVENTSCRIPTS)

PROJECT(EventScripts)

SET( prefix ${SCRIPTS_PATH}/${PROJECT_NAME} )

#sources -contains source files for our library.
SET( sources
Event_Arena_Seasons.cpp
Event_Brewfest.cpp
Event_Call_to_Arms.cpp
Event_Childrens_Week.cpp
Event_Darkmoon_Faire.cpp
Event_Day_of_the_Dead.cpp
Event_Edge_of_Madness.cpp
Event_Fishing_Extravaganza.cpp
Event_Fishing_Seasons.cpp
Event_Hallows_End.cpp
Event_Harvest_Festival.cpp
Event_Kaluak_Fishing_Derby.cpp
Event_L70ETC_Concert.cpp
Event_Leader_of_JinAlai.cpp
Event_Love_is_in_the_Air.cpp
Event_Lunar_Festival.cpp
Event_Midsummer_Fire_Festival.cpp
Event_New_Years_Eve.cpp
Event_Night_Elf_Mohawk.cpp
Event_Operation_Gnomeregan.cpp
Event_Pilgrims_Bounty.cpp
Event_Pirates_Day.cpp
Event_Wintergrasp.cpp
Event_WinterVeil.cpp
Event_World_PvP.cpp
Event_Worlds_End_Tavern.cpp
Event_VayrieTest.cpp
Event_Zalzanes_Fall.cpp
Setup.cpp
../Common/Base.cpp )

SET( headers
Event_Darkmoon_Faire.h
Setup.h
../Common/Base.h
../Common/EasyFunctions.h
../Common/Instance_Base.h )

foreach(src IN ITEMS ${sources} )
  SET( SRCS ${SRCS} ${prefix}/${src} )
endforeach(src)
foreach(src IN ITEMS ${headers} )
  SET( HEADERS ${HEADERS} ${prefix}/${src} )
endforeach(src)

include_directories( ${include_dirs} ${prefix} )
ADD_LIBRARY( ${PROJECT_NAME} SHARED ${SRCS} ${HEADERS} )
ADD_DEPENDENCIES( ${PROJECT_NAME} ${dependancies} )
target_link_libraries( ${PROJECT_NAME} ${linkages} )

include(PCHSupport)
IF( CMAKE_GENERATOR MATCHES Visual*)
	ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} Setup.h ${prefix}/Setup.cpp)
ENDIF()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${ASCEMU_SCRIPTLIB_PATH} LIBRARY DESTINATION ${ASCEMU_SCRIPTLIB_PATH} )
#Cleap up our variables(Not required.)
unset( prefix)
unset( SRCS )
unset( sources )