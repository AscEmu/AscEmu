# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Auth)

set(SRC_AUTH_FILES
    ${PATH_PREFIX}/BigNumber.cpp
    ${PATH_PREFIX}/BigNumber.h
    ${PATH_PREFIX}/MD5.cpp
    ${PATH_PREFIX}/MD5.h
    ${PATH_PREFIX}/Sha1.cpp
    ${PATH_PREFIX}/Sha1.h
    ${PATH_PREFIX}/WowCrypt.cpp
    ${PATH_PREFIX}/WowCrypt.hpp
)

source_group(Auth FILES ${SRC_AUTH_FILES})
unset(PATH_PREFIX)
