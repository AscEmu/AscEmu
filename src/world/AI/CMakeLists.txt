# Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX AI)

set(SRC_AI_FILES
   ${PATH_PREFIX}/MovementAI.cpp
   ${PATH_PREFIX}/MovementAI.h
)

source_group(AI FILES ${SRC_AI_FILES})
unset(PATH_PREFIX)