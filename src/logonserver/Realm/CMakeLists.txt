# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Realm)

set(SRC_REALM_FILES
    ${PATH_PREFIX}/RealmFlag.hpp
    ${PATH_PREFIX}/RealmManager.cpp
    ${PATH_PREFIX}/RealmManager.hpp
)

source_group(Realm FILES ${SRC_REALM_FILES})
unset(PATH_PREFIX)
