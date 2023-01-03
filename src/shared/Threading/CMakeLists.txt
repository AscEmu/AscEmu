# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Threading)

set(SRC_THREADING_FILES
    ${PATH_PREFIX}/AEThread.cpp
    ${PATH_PREFIX}/AEThread.h
    ${PATH_PREFIX}/AEThreadPool.cpp
    ${PATH_PREFIX}/AEThread.h
    ${PATH_PREFIX}/ConditionVariable.cpp
    ${PATH_PREFIX}/ConditionVariable.h
    ${PATH_PREFIX}/LegacyThreadBase.h
    ${PATH_PREFIX}/LegacyThreading.h
    ${PATH_PREFIX}/LegacyThreadPool.cpp
    ${PATH_PREFIX}/LegacyThreadPool.h
    ${PATH_PREFIX}/LockedQueue.h
    ${PATH_PREFIX}/Mutex.cpp
    ${PATH_PREFIX}/Mutex.h
    ${PATH_PREFIX}/Queue.h
    ${PATH_PREFIX}/ThreadState.h
)

source_group(Threading FILES ${SRC_THREADING_FILES})
unset(PATH_PREFIX)
