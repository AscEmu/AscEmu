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
    const size_t MAX_UPDATE_SIZE = 63000;

    std::mutex mtx_updateBuffer;
    std::mutex mtx_delayedPacketsLock;

    size_t m_compressionThreshold;

    ByteBuffer bCreationBuffer;
    uint32_t mCreationCount;
    ByteBuffer bUpdateBuffer;
    bool bProcessPending;
    ByteBuffer mOutOfRangeIds;
    uint32_t mUpdateCount;
    uint32_t mOutOfRangeIdCount;

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

    void queueDelayedPacket(WorldPacket* packet);
};
