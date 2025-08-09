/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <WorldPacket.h>

class Player;

typedef std::map<uint64_t, ByteBuffer*> SplineMap;

class UpdateManager
{
    const size_t m_maxUpdateSize = 63000;

    std::mutex m_mutexUpdateBuffer;
    std::mutex m_mutexDelayedPackets;

    size_t m_compressionThreshold;

    ByteBuffer m_creationBuffer;
    uint32_t m_creationCount;
    ByteBuffer m_updateBuffer;
    bool m_processPending;
    ByteBuffer m_outOfRangeIds;
    uint32_t m_updateCount;
    uint32_t m_outOfRangeIdCount;

    std::vector<std::unique_ptr<WorldPacket>> m_delayedPackets;
    Player* m_owner;

    size_t calculateBufferSize() const;
    bool readyForUpdate() const;

    void internalProcessPendingUpdates();
    void internalPushUpdatesIfBufferIsFull(size_t additionalDataSize);
    void internalSendDelayedPackets();
    void internalUpdateMapMgr();

public:
    UpdateManager(Player* owner, size_t compressionThreshold, size_t creationBufferInitialSize, size_t updateBufferInitialSize, size_t outOfRangeIdsInitialSize);

    void clearPendingUpdates();

    void pushCreationData(ByteBuffer* data, uint32_t updateCount);
    void pushOutOfRangeGuid(const WoWGuid& guid);
    void pushUpdateData(ByteBuffer* data, uint32_t updateCount);
    void processPendingUpdates();

    void queueDelayedPacket(std::unique_ptr<WorldPacket> packet);
};
