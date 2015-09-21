# Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>

# set up our project name
project(shared)

set(prefix ${ROOT_PATH}/src/shared)
set(sources
    CThreads.cpp 
    CrashHandler.cpp 
    Log.cpp 
    MersenneTwister.cpp 
    printStackTrace.cpp 
    Util.cpp 
    crc32.cpp 
    MapFile.cpp
    MapFileEntry.cpp
    TextFile.cpp
    FindFiles.cpp
    DynLib.cpp
    SysInfo.cpp
    PerformanceCounter.cpp
    Threading/Mutex.cpp
    Threading/Threading.h  
    Threading/ThreadPool.cpp 
    Threading/AtomicULong.cpp 
    Threading/AtomicFloat.cpp 
    Threading/AtomicCounter.cpp 
    Threading/AtomicBoolean.cpp 
    Threading/ConditionVariable.cpp
    ascemu_getopt.cpp 
    Auth/BigNumber.cpp 
    Auth/MD5.cpp 
    Auth/Sha1.cpp 
    Auth/WowCrypt.cpp 
    Config/Config.cpp 
    Database/Database.cpp 
    Database/MySQLDatabase.cpp 
    Database/CreateInterface.cpp 
    Network/CircularBuffer.cpp 
    Network/Socket.cpp
	)
	
set(headers 
	ascemu_getopt.h
	ascemuConfig.h
	Array.h
	AuthCodes.h
	ByteBuffer.h
	CallBack.h
	CircularQueue.h
	Common.h
	CrashHandler.h
	crc32.h
	CRefcounter.h
	CThreads.h
	Database/DatabaseEnv.h
	DynLib.hpp
	Errors.h
	FastQueue.h
	FindFiles.hpp
	FindFilesResult.hpp
	LocationVector.h
	Log.h
	MapFile.h
	MapFileEntry.h
	MersenneTwister.h
	PerformanceCounter.hpp
	PreallocatedQueue.h
	printStackTrace.h
	RC4Engine.h
	Singleton.h
	StackBuffer.h
	StackWalker.h
	StackTrace.h
	Storage.h
	SysInfo.hpp
	TextFile.h
	TLSObject.h
	Tokenizer.h
	Util.h
	WorldPacket.h
	WoWGuid.h
	Auth/BigNumber.h
	Auth/MD5.h
	Auth/Sha1.h
	Auth/WowCrypt.h
	Config/Config.h
	Config/ConfigEnv.h
	Database/Database.h
	Database/DataStore.h
	Database/Field.h
	Database/MySQLDatabase.h
	Network/CircularBuffer.h
	Network/ListenSocketFreeBSD.h
	Network/ListenSocketLinux.h
	Network/ListenSocketWin32.h
	Network/Network.h
	Network/Socket.h
	Network/SocketMgrFreeBSD.h
	Network/SocketMgrLinux.h
	Network/SocketMgrWin32.h
	Network/SocketOps.h
	Network/SocketDefines.h
	Threading/AtomicBoolean.h
	Threading/AtomicCounter.h
	Threading/AtomicFloat.h
	Threading/AtomicULong.h
	Threading/ConditionVariable.h
	Threading/Guard.h
	Threading/LockedQueue.h
	Threading/Mutex.h
	Threading/Queue.h
	Threading/RWLock.h
	Threading/Threading.h
	Threading/ThreadPool.h
	Threading/ThreadStarter.h
	)

if(WIN32)
   set(sources ${sources} Network/SocketMgrWin32.cpp Network/SocketOpsWin32.cpp Network/SocketWin32.cpp StackWalker.cpp StackTrace.cpp)
elseif(IS_FREEBSD OR APPLE)
   set(sources ${sources} Network/SocketMgrFreeBSD.cpp Network/SocketFreeBSD.cpp Network/SocketOpsFreeBSD.cpp)
else(WIN32)
   set(sources ${sources} Network/SocketMgrLinux.cpp Network/SocketLinux.cpp Network/SocketOpsLinux.cpp)
endif(WIN32)

foreach(src IN ITEMS ${sources})
   set(SRCS ${SRCS} ${prefix}/${src})
endforeach(src)
foreach(src IN ITEMS ${headers})
   set(HEADERS ${HEADERS} ${prefix}/${src})
endforeach(src)

include_directories(
   ${GLOBAL_INCLUDE_DIRS}
   ${DEPENDENCY_ROOT}/zlib
   )
if(IS_64BIT)
   link_directories(${DEPENDENCY_DLLS64})
else()
	link_directories(${DEPENDENCY_DLLS})
endif()
link_directories(${DEPENDENCY_LIBS})

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${MYSQL_LIBRARY} ${OPENSSL_LIBRARIES} ${EXTRA_LIBS})
