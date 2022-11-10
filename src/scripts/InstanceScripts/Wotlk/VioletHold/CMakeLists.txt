# Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Wotlk/VioletHold)

set(SRC_WOTLK_VIOLETHOLD_SCRIPTS_FILES
    ${PATH_PREFIX}/Instance_TheVioletHold.cpp
	${PATH_PREFIX}/Instance_TheVioletHold.hpp
	
	${PATH_PREFIX}/AzureSaboteur.cpp
	${PATH_PREFIX}/AzureSaboteur.hpp
	${PATH_PREFIX}/Portal_Intro.cpp
	${PATH_PREFIX}/Portal_Intro.hpp
	${PATH_PREFIX}/Portal_Common.cpp
	${PATH_PREFIX}/Portal_Common.hpp
	${PATH_PREFIX}/Portal_Elite.cpp
	${PATH_PREFIX}/Portal_Elite.hpp
	
	${PATH_PREFIX}/Erekem.cpp
	${PATH_PREFIX}/Erekem.hpp
	${PATH_PREFIX}/Ichron.cpp
	${PATH_PREFIX}/Ichron.hpp
	${PATH_PREFIX}/Cyangosa.cpp
	${PATH_PREFIX}/Cyangosa.hpp
	${PATH_PREFIX}/Lavanthor.cpp
	${PATH_PREFIX}/Lavanthor.hpp
	${PATH_PREFIX}/Moragg.cpp
	${PATH_PREFIX}/Moragg.hpp
	${PATH_PREFIX}/Xevozz.cpp
	${PATH_PREFIX}/Xevozz.hpp
	${PATH_PREFIX}/Zuramat.cpp
	${PATH_PREFIX}/Zuramat.hpp
)

source_group(Wotlk/VioletHold FILES ${SRC_WOTLK_VIOLETHOLD_SCRIPTS_FILES})
unset(PATH_PREFIX)
