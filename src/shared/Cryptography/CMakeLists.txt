# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Cryptography)

set(SRC_CRYPTOGRAPHY_FILES
    ${PATH_PREFIX}/AuthCodes.h
    ${PATH_PREFIX}/BigNumber.cpp
    ${PATH_PREFIX}/BigNumber.h
    ${PATH_PREFIX}/crc32.cpp
    ${PATH_PREFIX}/crc32.h
    ${PATH_PREFIX}/LogonCommDefines.h
    ${PATH_PREFIX}/MD5.cpp
    ${PATH_PREFIX}/MD5.h
    ${PATH_PREFIX}/Sha1.cpp
    ${PATH_PREFIX}/Sha1.h
    ${PATH_PREFIX}/WowCrypt.cpp
    ${PATH_PREFIX}/WowCrypt.hpp
)

source_group(Cryptography FILES ${SRC_CRYPTOGRAPHY_FILES})
unset(PATH_PREFIX)
