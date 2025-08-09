/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <cstdint>
#include <vector>

#include "UpdateManager.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Opcodes.hpp"
#include "Server/WorldSession.h"

UpdateManager::UpdateManager(Player* owner, size_t compressionThreshold, size_t creationBufferInitialSize, size_t updateBufferInitialSize, size_t outOfRangeIdsInitialSize)
    : 
    m_owner(owner),
    m_compressionThreshold(compressionThreshold),
    m_updateCount(0),
    m_creationCount(0),
    m_outOfRangeIdCount(0),
    m_processPending(false),
    m_creationBuffer(creationBufferInitialSize),
    m_updateBuffer(updateBufferInitialSize),
    m_outOfRangeIds(outOfRangeIdsInitialSize)
{
}

void UpdateManager::clearPendingUpdates()
{
    std::lock_guard update_guard(m_mutexUpdateBuffer);
    std::lock_guard packet_guard(m_mutexDelayedPackets);

    m_processPending = false;
    m_updateCount = 0;
    m_updateBuffer.clear();
}

void UpdateManager::pushCreationData(ByteBuffer* data, uint32_t updateCount)
{
    std::lock_guard update_guard(m_mutexUpdateBuffer);
    std::lock_guard packet_guard(m_mutexDelayedPackets);

    internalPushUpdatesIfBufferIsFull(data->size());

    m_creationCount += updateCount;
    m_creationBuffer.append(*data);

    internalUpdateMapMgr();
}

void UpdateManager::pushOutOfRangeGuid(const WoWGuid& guid)
{
    std::lock_guard update_guard(m_mutexUpdateBuffer);
    std::lock_guard packet_guard(m_mutexDelayedPackets);

    internalPushUpdatesIfBufferIsFull(static_cast<size_t>(8));

    m_outOfRangeIds << guid;
    ++m_outOfRangeIdCount;

    internalUpdateMapMgr();
}

void UpdateManager::pushUpdateData(ByteBuffer* data, uint32_t updateCount)
{
    std::lock_guard update_guard(m_mutexUpdateBuffer);
    std::lock_guard packet_guard(m_mutexDelayedPackets);
    
    internalPushUpdatesIfBufferIsFull(data->size());

    m_updateCount += updateCount;
    m_updateBuffer.append(*data);

    internalUpdateMapMgr();
}

void UpdateManager::processPendingUpdates()
{
    std::lock_guard update_guard(m_mutexUpdateBuffer);
    std::lock_guard packet_guard(m_mutexDelayedPackets);

    internalProcessPendingUpdates();

    // seems to be wrong but these packets needs to be send after "real" full login.
    m_owner->resendSpeed();
    m_owner->resetTimeSync();
    m_owner->sendTimeSync();
}

void UpdateManager::queueDelayedPacket(std::unique_ptr<WorldPacket> packet)
{
    std::lock_guard packet_guard(m_mutexDelayedPackets);

    m_delayedPackets.emplace_back(std::move(packet));
}

size_t UpdateManager::calculateBufferSize() const
{
    const size_t base_size = 10 + (m_outOfRangeIds.size() * 9);

    if (m_creationBuffer.size() > m_updateBuffer.size())
        return m_creationBuffer.size() + base_size;

    return m_updateBuffer.size() + base_size;
}

bool UpdateManager::readyForUpdate() const
{
    return m_creationBuffer.size() != 0 || m_updateBuffer.size() != 0 || m_outOfRangeIds.size() != 0;
}

void UpdateManager::internalProcessPendingUpdates()
{
    if (!readyForUpdate())
        return;

    ByteBuffer buffer(calculateBufferSize());

    if (m_creationBuffer.size() > 0 || m_outOfRangeIdCount > 0)
    {
#if VERSION_STRING >= Cata
        buffer << uint16_t(m_owner->GetMapId());
#endif

        if (m_outOfRangeIds.size() > 0)
            buffer << uint32_t(m_creationCount + 1);
        else
            buffer << uint32_t(m_creationCount);


#if VERSION_STRING <= TBC
        buffer << uint8_t(1);
#endif

        if (m_outOfRangeIdCount > 0)
        {
            buffer << uint8_t(UPDATETYPE_OUT_OF_RANGE_OBJECTS);
            buffer << uint32_t(m_outOfRangeIdCount);
            buffer.append(m_outOfRangeIds);
            m_outOfRangeIds.clear();
            m_outOfRangeIdCount = 0;
        }

        if (m_creationBuffer.size() > 0)
        {
            buffer.append(m_creationBuffer);
            m_creationBuffer.clear();
            m_creationCount = 0;
        }

        auto sent_packet = false;
#if VERSION_STRING < Cata
        if (buffer.wpos() > m_compressionThreshold)
            sent_packet = m_owner->compressAndSendUpdateBuffer(uint32_t(buffer.wpos()), buffer.contents());
#endif

        if (!sent_packet)
            m_owner->getSession()->OutPacket(SMSG_UPDATE_OBJECT, uint16_t(buffer.wpos()), buffer.contents());
}

    if (m_updateBuffer.size() > 0)
    {
        buffer.clear();

#if VERSION_STRING >= Cata
        buffer << uint16_t(m_owner->GetMapId());
#endif

        if (m_outOfRangeIds.size() > 0)
            buffer << uint32_t(m_updateCount + 1);
        else
            buffer << uint32_t(m_updateCount);

#if VERSION_STRING <= TBC
        buffer << uint8_t(1);
#endif

        buffer.append(m_updateBuffer);
        m_updateBuffer.clear();
        m_updateCount = 0;

        auto sent_packet = false;
#if VERSION_STRING < Cata
        if (buffer.wpos() > m_compressionThreshold)
            sent_packet = m_owner->compressAndSendUpdateBuffer(uint32_t(buffer.wpos()), buffer.contents());
#endif

        if (!sent_packet)
            m_owner->getSession()->OutPacket(SMSG_UPDATE_OBJECT, uint16_t(buffer.wpos()), buffer.contents());
    }

    m_processPending = false;
    internalSendDelayedPackets();
}

void UpdateManager::internalSendDelayedPackets()
{
    const auto session = m_owner->getSession();
    for (const auto& packet : m_delayedPackets)
        session->SendPacket(packet.get());

    m_delayedPackets.clear();
}

void UpdateManager::internalPushUpdatesIfBufferIsFull(size_t additionalDataSize)
{
    if (additionalDataSize + calculateBufferSize() >= m_maxUpdateSize)
        internalProcessPendingUpdates();
}

void UpdateManager::internalUpdateMapMgr()
{
    const auto mapMgr = m_owner->getWorldMap();
    if (mapMgr != nullptr && !m_processPending)
    {
        m_processPending = true;
        mapMgr->pushToProcessed(m_owner);
    }
}
