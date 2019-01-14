#include "UpdateManager.h"

#include <cstdint>
#include <vector>
#include "Map/MapMgr.h"
#include "Units/Players/Player.h"

using namespace std;

UpdateManager::UpdateManager(Player* owner, size_t compressionThreshold, size_t creationBufferInitialSize, size_t updateBufferInitialSize, size_t outOfRangeIdsInitialSize)
    : 
    m_owner(owner),
    m_compressionThreshold(compressionThreshold),
    mUpdateCount(0),
    mCreationCount(0),
    mOutOfRangeIdCount(0),
    bProcessPending(false),
    bCreationBuffer(creationBufferInitialSize),
    bUpdateBuffer(updateBufferInitialSize),
    mOutOfRangeIds(outOfRangeIdsInitialSize)
{
}

void UpdateManager::clearPendingUpdates()
{
    lock_guard<mutex> update_guard(mtx_updateBuffer);
    lock_guard<mutex> packet_guard(mtx_delayedPacketsLock);

    bProcessPending = false;
    mUpdateCount = 0;
    bUpdateBuffer.clear();
}

void UpdateManager::pushCreationData(ByteBuffer* data, uint32_t updateCount)
{
    lock_guard<mutex> update_guard(mtx_updateBuffer);
    lock_guard<mutex> packet_guard(mtx_delayedPacketsLock);

    internalPushUpdatesIfBufferIsFull(data->size());

    mCreationCount += updateCount;
    bCreationBuffer.append(*data);

    internalUpdateMapMgr();
}

void UpdateManager::pushOutOfRangeGuid(const WoWGuid & guid)
{
    lock_guard<mutex> update_guard(mtx_updateBuffer);
    lock_guard<mutex> packet_guard(mtx_delayedPacketsLock);

    internalPushUpdatesIfBufferIsFull(size_t(8));

    mOutOfRangeIds << guid;
    ++mOutOfRangeIdCount;

    internalUpdateMapMgr();
}

void UpdateManager::pushUpdateData(ByteBuffer * data, uint32_t updateCount)
{
    lock_guard<mutex> update_guard(mtx_updateBuffer);
    lock_guard<mutex> packet_guard(mtx_delayedPacketsLock);
    
    internalPushUpdatesIfBufferIsFull(data->size());

    mUpdateCount += updateCount;
    bUpdateBuffer.append(*data);

    internalUpdateMapMgr();
}

void UpdateManager::processPendingUpdates()
{
    lock_guard<mutex> update_guard(mtx_updateBuffer);
    lock_guard<mutex> packet_guard(mtx_delayedPacketsLock);

    internalProcessPendingUpdates();
    m_owner->resendSpeed();
}

void UpdateManager::queueDelayedPacket(WorldPacket * packet)
{
    lock_guard<mutex> packet_guard(mtx_delayedPacketsLock);

    m_delayedPackets.push_back(unique_ptr<WorldPacket>(packet));
}

size_t UpdateManager::calculateBufferSize() const
{
    const size_t base_size = 10 + (mOutOfRangeIds.size() * 9);

    if (bCreationBuffer.size() > bUpdateBuffer.size())
    {
        return bCreationBuffer.size() + base_size;
    }

    return bUpdateBuffer.size() + base_size;
}

bool UpdateManager::readyForUpdate() const
{
    return bCreationBuffer.size() != 0
        || bUpdateBuffer.size() != 0
        || mOutOfRangeIds.size() != 0;
}

void UpdateManager::internalProcessPendingUpdates()
{
    if (!readyForUpdate())
    {
        return;
    }

    ByteBuffer buffer(calculateBufferSize());

    if (bCreationBuffer.size() > 0 || mOutOfRangeIdCount > 0)
    {
#if VERSION_STRING >= Cata
        buffer << uint16_t(m_owner->GetMapId());
#endif

        if (mOutOfRangeIds.size() > 0)
        {
            buffer << uint32_t(mCreationCount + 1);
        }
        else
        {
            buffer << uint32_t(mCreationCount);
        }


#if VERSION_STRING <= TBC
        buffer << uint8_t(1);
#endif

        if (mOutOfRangeIdCount > 0)
        {
            buffer << uint8_t(UPDATETYPE_OUT_OF_RANGE_OBJECTS);
            buffer << uint32_t(mOutOfRangeIdCount);
            buffer.append(mOutOfRangeIds);
            mOutOfRangeIds.clear();
            mOutOfRangeIdCount = 0;
        }

        if (bCreationBuffer.size() > 0)
        {
            buffer.append(bCreationBuffer);
            bCreationBuffer.clear();
            mCreationCount = 0;
        }

        auto sent_packet = false;
#if VERSION_STRING < Cata
        if (buffer.wpos() > m_compressionThreshold)
        {
            sent_packet = m_owner->CompressAndSendUpdateBuffer(uint32_t(buffer.wpos()), buffer.contents());
        }
#endif

        if (!sent_packet)
        {
            m_owner->GetSession()->OutPacket(SMSG_UPDATE_OBJECT, uint16_t(buffer.wpos()), buffer.contents());
        }
}

    if (bUpdateBuffer.size() > 0)
    {
        buffer.clear();

#if VERSION_STRING >= Cata
        buffer << uint16_t(m_owner->GetMapId());
#endif

        if (mOutOfRangeIds.size() > 0)
        {
            buffer << uint32_t(mUpdateCount + 1);
        }
        else
        {
            buffer << uint32_t(mUpdateCount);
        }

#if VERSION_STRING <= TBC
        buffer << uint8_t(1);
#endif

        buffer.append(bUpdateBuffer);
        bUpdateBuffer.clear();
        mUpdateCount = 0;

        auto sent_packet = false;
#if VERSION_STRING < Cata
        if (buffer.wpos() > m_compressionThreshold)
        {
            sent_packet = m_owner->CompressAndSendUpdateBuffer(uint32_t(buffer.wpos()), buffer.contents());
        }
#endif

        if (!sent_packet)
        {
            m_owner->GetSession()->OutPacket(SMSG_UPDATE_OBJECT, uint16_t(buffer.wpos()), buffer.contents());
        }
    }

    bProcessPending = false;
    internalSendDelayedPackets();
}

void UpdateManager::internalSendDelayedPackets()
{
    const auto session = m_owner->GetSession();
    for (const auto& packet : m_delayedPackets)
    {
        session->SendPacket(packet.get());
    }

    m_delayedPackets.clear();
}

void UpdateManager::internalPushUpdatesIfBufferIsFull(size_t additionalDataSize)
{
    if (additionalDataSize + calculateBufferSize() >= MAX_UPDATE_SIZE)
    {
        internalProcessPendingUpdates();
    }
}

void UpdateManager::internalUpdateMapMgr()
{
    const auto mapMgr = m_owner->GetMapMgr();
    if (mapMgr != nullptr && !bProcessPending)
    {
        bProcessPending = true;
        mapMgr->PushToProcessed(m_owner);
    }
}
