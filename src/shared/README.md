# shared

## Introduction
Shared project for functions/structures/classes used by world and logon project.
Do not add code which is not used by world AND logonserver.

## State
### Legacy Code
Legacy code is mostly released under AGPL and highly outdated and can be rewritten with modern C++.
Keep legacy code inside a AGPL file. Do NOT copy any legacy code to a MIT file!

Keep in mind that legacy code was written mostly in C-style/c++98 and horrible outdated.

### New Code
New code has to be placed inside a MIT file. The current standard is C++14 and newer.

## List of Legacy files
### Not rewritten - yet
* 2014 | Network/SocketDefines.h
* 2014 | Network/NetworkIncludes.hpp
* 2014 | CommonTypes.hpp
* 2014 | CommonHelpers.hpp
* 2014 | CommonDefines.hpp
* 2008 | WoWGuid.h
* 2008 | WorldPacket.h
* 2008 | Threading/LegacyThreadPool.h
* 2008 | Threading/LegacyThreadPool.cpp
* 2008 | Threading/LegacyThreadBase.h
* 2008 | Threading/LegacyThreading.h
* 2008 | Threading/Queue.h
* 2008 | Threading/LockedQueue.h
* 2008 | Threading/ConditionVariable.h
* 2008 | Threading/ConditionVariable.cpp
* 2008 | PreallocatedQueue.h
* 2008 | LocationVector.cpp
* 2008 | LocationVector.h
* 2008 | DynLib.hpp
* 2008 | DynLib.cpp
* 2008 | Database/MySQLDatabase.h
* 2008 | Database/Database.h
* 2008 | Database/Database.cpp
* 2008 | Database/CreateInterface.cpp
* 2008 | CThreads.h
* 2008 | CThreads.cpp
* 2008 | Debugging/Errors.h
* 2008 | Debugging/CrashHandler.h
* 2008 | Debugging/CrashHandler.cpp
* 2008 | Config/Config.h
* 2008 | Config/Config.cpp
* 2008 | Utilities/CallBack.h
* 2008 | ByteBuffer.h
* 2008 | Cryptography/AuthCodes.h
* 2008 | Cryptography/BigNumber.cpp
* 2007 | Network/EPOLL/ListenSocketLinux.h
* 2007 | Network/EPOLL/SocketLinux.cpp
* 2007 | Network/EPOLL/SocketMgrLinux.cpp
* 2007 | Network/EPOLL/SocketMgrLinux.h
* 2007 | Network/EPOLL/SocketOpsLinux.cpp
* 2007 | Network/IOCP/ListenSocketWin32.h
* 2007 | Network/IOCP/SocketMgrWin32.cpp
* 2007 | Network/IOCP/SocketMgrWin32.h
* 2007 | Network/IOCP/SocketOpsWin32.cpp
* 2007 | Network/IOCP/SocketWin32.cpp
* 2007 | Network/KQUEUE/ListenSocketFreeBSD.h
* 2007 | Network/KQUEUE/SocketFreeBSD.cpp
* 2007 | Network/KQUEUE/SocketMgrFreeBSD.cpp
* 2007 | Network/KQUEUE/SocketMgrFreeBSD.h
* 2007 | Network/KQUEUE/SocketOpsFreeBSD.cpp
* 2007 | Network/CircularBuffer.cpp
* 2007 | Network/CircularBuffer.h
* 2007 | Network/Network.h
* 2007 | Network/Socket.cpp
* 2007 | Network/Socket.h
* 2007 | Network/SocketOps.h
* 2007 | Cryptography/BigNumber.h
* 2005 | SysInfo.hpp
* 2005 | SysInfo.cpp
* 2005 | PerformanceCounter.hpp
* 2005 | PerformanceCounter.cpp
* 2005 | Database/MySQLDatabase.cpp
* 2005 | CircularQueue.h
* 2005 | StackWalker.h
* 2005 | StackWalker.cpp

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

### Removed on 23 Nov 2017
* 2008 | ascemu_getopt.cpp
* 2007 | ascemu_getopt.h

### Removed on 23 Dez 2017
* 2008 | MersenneTwister.h
* 2008 | MersenneTwister.cpp

### Removed between 2018 - 2020
* 2008 | Threading/AtomicULong.h
* 2008 | Threading/AtomicULong.cpp
* 2008 | Threading/AtomicFloat.h
* 2008 | Threading/AtomicFloat.cpp
* 2008 | Threading/AtomicCounter.h
* 2008 | Threading/AtomicCounter.cpp
* 2008 | Threading/AtomicBoolean.h
* 2008 | Threading/AtomicBoolean.cpp

### Removed on 11 Jul 2019
* 2008 | FindFilesResult.hpp
* 2008 | FindFiles.hpp
* 2008 | FindFiles.cpp

### Removed on 17 Nov 2019
* 2008 | Singleton.h

### Removed on 20 Apr 2020
* 2008 | Database/Field.h

### Removed on 28 Jan 2021
* 2008 | Threading/Guard.h
* 2001 | printStackTrace.h
* 2001 | printStackTrace.cpp
* 2001 | MapFileEntry.h
* 2001 | MapFileEntry.cpp
* 2001 | MapFile.h
* 2001 | MapFile.cpp
* 2001 | StackTrace.h
* 2001 | StackTrace.cpp

### Removed on 29 Jan 2021
* 2001 | TextFile.h
* 2001 | TextFile.cpp
* 2008 | StackBuffer.h
* 2001 | Array.h

### Removed on 30 Jan 2021
* 2005 | CRefcounter.h
* 2008 | Database/DatabaseEnv.h

### Removed on 02 Feb 2021
* 2008 | Threading/RWLock.h

### Removed on 10 May 2022
* 2008 | TLSObject.h

### Removed on 07 Jun 2022
* 2014 | AscemuServerDefines.hpp

### Removed on 24 Aug 2023
* 2008 | FastQueue.h

### Removed on 16 Sep 2023
* 2021 | Database/DatabaseCommon.hpp

### Removed on 09 Mar 2024
* 1996 | crc32.h
* 1996 | crc32.cpp

### Removed on 03 Sep 2024
* 2007 | Cryptography/Sha1.h
* 2007 | Cryptography/MD5.h
* 2007 | Cryptography/RC4Engine.h

### Removed on 04 Sep 2024
* 2008 | Common.Legacy.h
* 2008 | Threading/Mutex.h

### New files
* 2015 | Exceptions/PlayerExceptions.hpp
* 2015 | Exceptions/Exceptions.hpp
* 2016 | git_version.hpp
* 2017 | ByteConverter.h
* 2017 | Common.hpp
* 2017 | Logging/Log.cpp
* 2017 | Logging/Log.hpp
* 2017 | Logging/LoggerDefines.hpp
* 2017 | Utilities/Util.cpp
* 2017 | Utilities/Util.hpp
* 2017 | Cryptography/LogonCommDefines.h
* 2017 | Cryptography/WowCrypt.cpp
* 2017 | Cryptography/WowCrypt.hpp
* 2017 | Threading/AEThread.cpp
* 2017 | Threading/AEThread.h
* 2017 | Threading/ThreadState.h
* 2018 | Threading/AEThreadPool.cpp
* 2018 | Threading/AEThreadPool.h
* 2020 | Database/Field.hpp
* 2020 | Database/DatabaseUpdater.cpp
* 2020 | Database/DatabaseUpdater.hpp
* 2021 | FactoryHolder.h
* 2021 | Logging/Logger.cpp
* 2021 | Logging/Logger.hpp
* 2021 | Logging/MessageType.hpp
* 2021 | Logging/Severity.hpp
* 2021 | ObjectRegistry.h
* 2021 | Utilities/Strings.cpp
* 2021 | Utilities/Strings.hpp
* 2022 | AEVersion.hpp
* 2023 | pchShared.hpp
* 2023 | ThreadSafeQueue.hpp
* 2023 | CommonTime.hpp
* 2023 | CommonFilesystem.hpp
* 2024 | Cryptography/Sha1.hpp
* 2024 | Cryptography/MD5.hpp
* 2024 | Cryptography/RC4.hpp
* 2024 | Threading/Mutex.hpp
* 2024 | Threading/Mutex.cpp
