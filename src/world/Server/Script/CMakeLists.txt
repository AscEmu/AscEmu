# Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Server/Script)

set(SRC_SERVER_SCRIPT_FILES
   ${PATH_PREFIX}/CreatureAIScript.cpp
   ${PATH_PREFIX}/CreatureAIScript.h
   ${PATH_PREFIX}/ScriptMgr.cpp
   ${PATH_PREFIX}/ScriptMgr.h
   ${PATH_PREFIX}/ScriptSetup.h
)

source_group(Server\\Script FILES ${SRC_SERVER_SCRIPT_FILES})
unset(PATH_PREFIX)
