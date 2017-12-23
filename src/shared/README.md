# shared

## Introduction
Shared project for functions/structures/classes used by world and logon project.
Do not add code which is not used by world- AND logonserver.

## State
### Legacy Code
Legacy code is mostly released under AGPL and highly outdated and can be rewritten with modern C++.
Keep legacy code inside a AGPL file. Do NOT copy any legacy code to a MIT file!

Keep in mind that legacy code was written mostly in C-style/c++98 and horrible outdated.

### New Code
New code has to be placed inside a MIT file. The current max standard is C++14.

## List of Legacy files
### Not rewritten - yet
* 2015 | Exceptions/PlayerExceptions.hpp
* 2015 | Exceptions/Exceptions.hpp
* 2014 | Network/SocketDefines.h
* 2014 | Network/NetworkIncludes.hpp
* 2014 | CommonTypes.hpp
* 2014 | CommonHelpers.hpp
* 2014 | CommonDefines.hpp
* 2014 | AscemuServerDefines.hpp
* 2008 | WoWGuid.h
* 2008 | WorldPacket.h
* 2008 | TLSObject.h
* 2008 | Threading/ThreadStarter.h
* 2008 | Threading/ThreadPool.h
* 2008 | Threading/ThreadPool.cpp
* 2008 | Threading/Threading.h
* 2008 | Threading/Threading.h
* 2008 | Threading/RWLock.h
* 2008 | Threading/Queue.h
* 2008 | Threading/Mutex.h
* 2008 | Threading/Mutex.cpp
* 2008 | Threading/LockedQueue.h
* 2008 | Threading/Guard.h
* 2008 | Threading/ConditionVariable.h
* 2008 | Threading/ConditionVariable.cpp
* 2008 | Threading/AtomicULong.h
* 2008 | Threading/AtomicULong.cpp
* 2008 | Threading/AtomicFloat.h
* 2008 | Threading/AtomicFloat.cpp
* 2008 | Threading/AtomicCounter.h
* 2008 | Threading/AtomicCounter.cpp
* 2008 | Threading/AtomicBoolean.h
* 2008 | Threading/AtomicBoolean.cpp
* 2008 | StackBuffer.h
* 2008 | Singleton.h
* 2008 | PreallocatedQueue.h
* 2008 | LocationVector.h
* 2008 | FindFilesResult.hpp
* 2008 | FindFiles.hpp
* 2008 | FindFiles.cpp
* 2008 | FastQueue.h
* 2008 | Errors.h
* 2008 | DynLib.hpp
* 2008 | DynLib.cpp
* 2008 | Database/MySQLDatabase.h
* 2008 | Database/Field.h
* 2008 | Database/DatabaseEnv.h
* 2008 | Database/Database.h
* 2008 | Database/Database.cpp
* 2008 | Database/CreateInterface.cpp
* 2008 | CThreads.h
* 2008 | CThreads.cpp
* 2008 | CrashHandler.h
* 2008 | CrashHandler.cpp
* 2008 | Config/Config.h
* 2008 | Config/Config.cpp
* 2008 | Common.Legacy.h
* 2008 | CallBack.h
* 2008 | ByteBuffer.h
* 2008 | AuthCodes.h
* 2008 | Auth/Sha1.cpp
* 2008 | Auth/MD5.cpp
* 2008 | Auth/BigNumber.cpp
* 2008 | ascemu_getopt.cpp
* 2007 | RC4Engine.h
* 2007 | Network/SocketWin32.cpp
* 2007 | Network/SocketOpsWin32.cpp
* 2007 | Network/SocketOpsLinux.cpp
* 2007 | Network/SocketOpsFreeBSD.cpp
* 2007 | Network/SocketOps.h
* 2007 | Network/SocketMgrWin32.h
* 2007 | Network/SocketMgrWin32.cpp
* 2007 | Network/SocketMgrLinux.h
* 2007 | Network/SocketMgrLinux.cpp
* 2007 | Network/SocketMgrFreeBSD.h
* 2007 | Network/SocketMgrFreeBSD.cpp
* 2007 | Network/SocketLinux.cpp
* 2007 | Network/SocketFreeBSD.cpp
* 2007 | Network/Socket.h
* 2007 | Network/Socket.cpp
* 2007 | Network/Network.h
* 2007 | Network/ListenSocketWin32.h
* 2007 | Network/ListenSocketLinux.h
* 2007 | Network/ListenSocketFreeBSD.h
* 2007 | Network/CircularBuffer.h
* 2007 | Network/CircularBuffer.cpp
* 2007 | Auth/Sha1.h
* 2007 | Auth/MD5.h
* 2007 | Auth/BigNumber.h
* 2007 | ascemu_getopt.h
* 2005 | SysInfo.hpp
* 2005 | SysInfo.cpp
* 2005 | StackWalker.h
* 2005 | StackWalker.cpp
* 2005 | PerformanceCounter.hpp
* 2005 | PerformanceCounter.cpp
* 2005 | Database/MySQLDatabase.cpp
* 2005 | CRefcounter.h
* 2005 | CircularQueue.h
* 2001 | TextFile.h
* 2001 | TextFile.cpp
* 2001 | StackTrace.h
* 2001 | StackTrace.cpp
* 2001 | printStackTrace.h
* 2001 | printStackTrace.cpp
* 2001 | MapFileEntry.h
* 2001 | MapFileEntry.cpp
* 2001 | MapFile.h
* 2001 | MapFile.cpp
* 2001 | Array.h
* 1996 | crc32.h
* 1996 | crc32.cpp

### Ready to remove

### Removed on 08 Jan 2017
* 2008 | Tokenizer.h
* 2008 | Config/ConfigEnv.h
* 2008 | Database/DataStore.h
* 2008 | Util.Legacy.h
* 2008 | Util.Legacy.cpp

### Removed on 16 Sep 2017
* 2008 | Log.Legacy.h
* 2008 | Log.Legacy.cpp
* 2005 | Timer.h

### Removed on 23 Dez 2017
* 2008 | MersenneTwister.h
* 2008 | MersenneTwister.cpp

### New files
* 2017 | Common.hpp
* 2017 | Log.cpp
* 2017 | Log.hpp
* 2017 | LogDefines.hpp
* 2017 | Util.cpp
* 2017 | Util.hpp
* 2017 | Auth/WowCrypt.cpp
* 2017 | Auth/WowCrypt.h